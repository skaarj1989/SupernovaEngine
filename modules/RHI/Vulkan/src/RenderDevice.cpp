#include "rhi/RenderDevice.hpp"
#include "FeatureBuilder.hpp"
#include "DebugCallback.hpp"
#include "ShaderReflection.hpp"
#include "VkCheck.hpp"

#include "math/Hash.hpp"

#include "glm/common.hpp" // clamp

#define _USE_VMA_LOGGER 0
#if _USE_VMA_LOGGER
#  include "spdlog/sinks/dup_filter_sink.h"
#  include "spdlog/sinks/stdout_color_sinks.h"
#endif
#include "spdlog/spdlog.h"

#include <ranges>

// In case of a crash in NSIGHT Graphics, comment the following line.
#define _USE_VALIDATION_LAYERS _DEBUG

//
// std::hash specializations:
//

namespace std {

template <> struct hash<VkDescriptorSetLayoutBinding> {
  auto operator()(const VkDescriptorSetLayoutBinding &v) const noexcept {
    size_t h{0};
    hashCombine(h, v.binding, v.descriptorType, v.descriptorCount,
                v.stageFlags);
    return h;
  }
};
template <> struct hash<VkPushConstantRange> {
  auto operator()(const VkPushConstantRange &v) const noexcept {
    size_t h{0};
    hashCombine(h, v.offset, v.size, v.stageFlags);
    return h;
  }
};

template <> struct hash<rhi::SamplerInfo> {
  auto operator()(const rhi::SamplerInfo &v) const noexcept {
    size_t h{0};
    hashCombine(h, v.magFilter, v.minFilter, v.mipmapMode, v.addressModeS,
                v.addressModeT, v.addressModeR, v.maxAnisotropy.has_value(),
                v.maxAnisotropy ? *v.maxAnisotropy : 0.0f,
                v.compareOp.has_value(),
                v.compareOp ? *v.compareOp : rhi::CompareOp::Never,
                int32_t(v.minLod), int32_t(v.maxLod), int32_t(v.borderColor));
    return h;
  }
};

} // namespace std

namespace rhi {

namespace {

#define _TARGET_VERSION VK_API_VERSION_1_3
constexpr auto kTargetVersion = _TARGET_VERSION;

[[nodiscard]] auto enumerateInstanceLayers() {
  uint32_t count{0};
  VK_CHECK(vkEnumerateInstanceLayerProperties(&count, nullptr));
  std::vector<VkLayerProperties> layerProperties(count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&count, layerProperties.data()));
  return layerProperties;
}
[[nodiscard]] const VkLayerProperties *
queryInstanceLayer(std::span<const VkLayerProperties> layers,
                   const std::string_view name) {
  for (const auto &properties : layers) {
    if (properties.layerName == name) return &properties;
  }
  return nullptr;
}

[[nodiscard]] auto enumerateInstanceExtensions() {
  uint32_t count{0};
  VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
  std::vector<VkExtensionProperties> extensionProperties(count);
  VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                                  extensionProperties.data()));
  return extensionProperties;
}
[[nodiscard]] std::optional<uint32_t>
queryExtension(std::span<const VkExtensionProperties> extensions,
               const std::string_view name) {
  for (const auto &properties : extensions) {
    if (properties.extensionName == name) return properties.specVersion;
  }
  return std::nullopt;
}

[[nodiscard]] auto enumeratePhysicalDevices(const VkInstance instance) {
  uint32_t count{0};
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, nullptr));
  assert(count > 0);

  std::vector<VkPhysicalDevice> physicalDevices(count);
  VK_CHECK(
    vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()));
  return physicalDevices;
}

[[nodiscard]] auto getPhysicalDevices(const VkInstance instance) {
  std::vector<PhysicalDevice> physicalDevices;
  std::ranges::transform(
    enumeratePhysicalDevices(instance), std::back_inserter(physicalDevices),
    [](VkPhysicalDevice h) {
      PhysicalDevice physicalDevice{.handle = h};
      vkGetPhysicalDeviceProperties(physicalDevice.handle,
                                    &physicalDevice.properties);
      vkGetPhysicalDeviceFeatures(physicalDevice.handle,
                                  &physicalDevice.features);
      vkGetPhysicalDeviceMemoryProperties(physicalDevice.handle,
                                          &physicalDevice.memoryProperties);
      return physicalDevice;
    });

  return physicalDevices;
}

[[nodiscard]] auto findQueueFamily(const VkPhysicalDevice physicalDevice,
                                   const VkQueueFlags requestedTypes) {
  uint32_t count{0};
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  assert(count > 0);
  std::vector<VkQueueFamilyProperties> queues(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
                                           queues.data());

  for (decltype(count) i{0}; i < count; ++i)
    if ((queues[i].queueFlags & requestedTypes) == requestedTypes) return i;

  return VK_QUEUE_FAMILY_IGNORED;
}

[[nodiscard]] auto
enumeratePhysicalDeviceExtensions(const VkPhysicalDevice device) {
  uint32_t count{0};
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> properties(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
                                       properties.data());
  return properties;
}

[[nodiscard]] constexpr auto makeAllocationFlags(const AllocationHints hints) {
  VkFlags flags{0};
  if (bool(hints & AllocationHints::MinMemory))
    flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
  if (bool(hints & AllocationHints::SequentialWrite))
    flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
             VMA_ALLOCATION_CREATE_MAPPED_BIT;
  return flags;
}

} // namespace

//
// RenderDevice class:
//

RenderDevice::RenderDevice() : RenderDevice{defaultDeviceSelector} {}
RenderDevice::RenderDevice(
  const PhysicalDeviceSelector &physicalDeviceSelector) {
  _createInstance();
  _selectPhysicalDevice(physicalDeviceSelector);
  assert(m_physicalDevice.handle != VK_NULL_HANDLE);
#ifdef _DEBUG
  m_debugMessenger = createDebugMessenger(m_instance);
#endif

  const auto genericQueueFamilyIndex = findQueueFamily(
    m_physicalDevice.handle,
    VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
  if (genericQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
    throw std::runtime_error{"Could not find suitable queue family."};

  _createLogicalDevice(genericQueueFamilyIndex);
  _createMemoryAllocator();

  const VkCommandPoolCreateInfo commandPoolInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = genericQueueFamilyIndex,
  };
  VK_CHECK(vkCreateCommandPool(m_logicalDevice, &commandPoolInfo, nullptr,
                               &m_commandPool));

  const VkPipelineCacheCreateInfo pipelineCacheInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
  };
  VK_CHECK(vkCreatePipelineCache(m_logicalDevice, &pipelineCacheInfo, nullptr,
                                 &m_pipelineCache));

  m_samplers.reserve(100);
  m_descriptorSetLayouts.reserve(100);
  m_pipelineLayouts.reserve(100);
}
RenderDevice::~RenderDevice() {
  if (m_instance == VK_NULL_HANDLE) return;

  waitIdle();

  m_garbageCollector.clear();

  for (auto [_, h] : m_pipelineLayouts)
    vkDestroyPipelineLayout(m_logicalDevice, h, nullptr);
  for (auto [_, h] : m_descriptorSetLayouts)
    vkDestroyDescriptorSetLayout(m_logicalDevice, h, nullptr);

  for (auto [_, sampler] : m_samplers)
    vkDestroySampler(m_logicalDevice, sampler, nullptr);

  vkDestroyPipelineCache(m_logicalDevice, m_pipelineCache, nullptr);
  m_pipelineCache = VK_NULL_HANDLE;
  vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
  m_commandPool = VK_NULL_HANDLE;

  if (m_memoryAllocator) {
    vmaDestroyAllocator(m_memoryAllocator);
    m_memoryAllocator = VK_NULL_HANDLE;
  }
  vkDestroyDevice(m_logicalDevice, nullptr);
  m_logicalDevice = VK_NULL_HANDLE;
  m_genericQueue = VK_NULL_HANDLE;

  if (m_debugMessenger) {
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    m_debugMessenger = VK_NULL_HANDLE;
  }
  vkDestroyInstance(m_instance, nullptr);
  m_instance = VK_NULL_HANDLE;

  m_physicalDevice = {};

  gladLoaderUnloadVulkan();
}

RenderDevice::operator bool() const { return m_instance != VK_NULL_HANDLE; }

std::string RenderDevice::getName() const {
  const auto v = m_physicalDevice.properties.apiVersion;
  return std::format("Vulkan {}.{}.{}", VK_API_VERSION_MAJOR(v),
                     VK_API_VERSION_MINOR(v), VK_API_VERSION_PATCH(v));
}

PhysicalDeviceInfo RenderDevice::getPhysicalDeviceInfo() const {
  return PhysicalDeviceInfo{
    .vendorId = m_physicalDevice.properties.vendorID,
    .deviceId = m_physicalDevice.properties.deviceID,
    .deviceName = m_physicalDevice.properties.deviceName,
  };
}

const VkPhysicalDeviceLimits &RenderDevice::getDeviceLimits() const {
  assert(m_physicalDevice.handle != VK_NULL_HANDLE);
  return m_physicalDevice.properties.limits;
}
const VkPhysicalDeviceFeatures &RenderDevice::getDeviceFeatures() const {
  assert(m_physicalDevice.handle != VK_NULL_HANDLE);
  return m_physicalDevice.features;
}

VkFormatProperties
RenderDevice::getFormatProperties(const PixelFormat pixelFormat) const {
  assert(m_physicalDevice.handle != VK_NULL_HANDLE);
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(m_physicalDevice.handle,
                                      VkFormat(pixelFormat), &props);
  return props;
}

Swapchain RenderDevice::createSwapchain(const os::Window &window,
                                        const Swapchain::Format format,
                                        const VerticalSync vsync) {
  assert(m_logicalDevice != VK_NULL_HANDLE);
  return Swapchain{
    m_instance, m_physicalDevice.handle, m_logicalDevice, window, format, vsync,
  };
}

VkFence RenderDevice::createFence(const bool signaled) {
  assert(m_logicalDevice != VK_NULL_HANDLE);
  const VkFenceCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
  };
  VkFence fence{VK_NULL_HANDLE};
  VK_CHECK(vkCreateFence(m_logicalDevice, &createInfo, nullptr, &fence));
  return fence;
}
VkSemaphore RenderDevice::createSemaphore() {
  assert(m_logicalDevice != VK_NULL_HANDLE);
  const VkSemaphoreCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .flags = 0, // Always 0, reserved for future use.
  };
  VkSemaphore semaphore{VK_NULL_HANDLE};
  VK_CHECK(
    vkCreateSemaphore(m_logicalDevice, &createInfo, nullptr, &semaphore));
  return semaphore;
}

Buffer RenderDevice::createStagingBuffer(const VkDeviceSize size,
                                         const void *data) {
  assert(m_memoryAllocator != nullptr);
  Buffer stagingBuffer{
    m_memoryAllocator,
    size,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    makeAllocationFlags(AllocationHints::SequentialWrite),
    VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
  };
  if (data) {
    auto *mappedPtr = stagingBuffer.map();
    std::memcpy(mappedPtr, data, size);
    stagingBuffer.unmap();
  }
  return stagingBuffer;
}

VertexBuffer
RenderDevice::createVertexBuffer(const Buffer::Stride stride,
                                 const VkDeviceSize capacity,
                                 const AllocationHints allocationHint) {
  assert(m_memoryAllocator != nullptr);
  return VertexBuffer{
    Buffer{
      m_memoryAllocator,
      stride * capacity,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      makeAllocationFlags(allocationHint),
      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    },
    stride,
  };
}
IndexBuffer
RenderDevice::createIndexBuffer(const IndexType indexType,
                                const VkDeviceSize capacity,
                                const AllocationHints allocationHint) {
  assert(m_memoryAllocator != nullptr);
  return IndexBuffer{
    Buffer{
      m_memoryAllocator,
      uint8_t(indexType) * capacity,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      makeAllocationFlags(allocationHint),
      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    },
    indexType};
}
UniformBuffer
RenderDevice::createUniformBuffer(const VkDeviceSize size,
                                  const AllocationHints allocationHint) {
  assert(m_memoryAllocator != nullptr);
  return UniformBuffer{
    m_memoryAllocator,
    size,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    makeAllocationFlags(allocationHint),
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  };
}
StorageBuffer
RenderDevice::createStorageBuffer(const VkDeviceSize size,
                                  const AllocationHints allocationHint) {
  assert(m_memoryAllocator != nullptr);
  return StorageBuffer{
    m_memoryAllocator,
    size,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    makeAllocationFlags(allocationHint),
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  };
}

std::string RenderDevice::getMemoryStats() const {
  assert(m_memoryAllocator != nullptr);
  char *stats{nullptr};
  vmaBuildStatsString(m_memoryAllocator, &stats, VK_TRUE);
  std::string s{stats};
  vmaFreeStatsString(m_memoryAllocator, stats);
  return s;
}

std::pair<std::size_t, VkDescriptorSetLayout>
RenderDevice::createDescriptorSetLayout(
  const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
  assert(m_logicalDevice != VK_NULL_HANDLE);

  std::size_t hash{0};
  for (const auto &b : bindings)
    hashCombine(hash, b);

  if (const auto it = m_descriptorSetLayouts.find(hash);
      it != m_descriptorSetLayouts.cend()) {
    return {hash, it->second};
  } else {
    const VkDescriptorSetLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = uint32_t(bindings.size()),
      .pBindings = bindings.data(),
    };
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &createInfo, nullptr,
                                         &descriptorSetLayout));

    const auto &[inserted, _] =
      m_descriptorSetLayouts.emplace(hash, descriptorSetLayout);
    return {hash, inserted->second};
  }
}

PipelineLayout
RenderDevice::createPipelineLayout(const PipelineLayoutInfo &layoutInfo) {
  assert(m_logicalDevice != VK_NULL_HANDLE);

  std::size_t hash{0};
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts(
    kMinNumDescriptorSets);

  for (auto [set, bindings] :
       std::views::enumerate(layoutInfo.descriptorSets)) {
    for (const auto &binding : bindings) {
      hashCombine(hash, set, binding);
    }
    auto [_, handle] = createDescriptorSetLayout(bindings);
    descriptorSetLayouts[set] = handle;
  }
  for (const auto &range : layoutInfo.pushConstantRanges)
    hashCombine(hash, range);

  // --

  if (const auto it = m_pipelineLayouts.find(hash);
      it != m_pipelineLayouts.cend()) {
    return PipelineLayout{it->second, std::move(descriptorSetLayouts)};
  }
  const VkPipelineLayoutCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = uint32_t(descriptorSetLayouts.size()),
    .pSetLayouts = descriptorSetLayouts.data(),
    .pushConstantRangeCount = uint32_t(layoutInfo.pushConstantRanges.size()),
    .pPushConstantRanges = layoutInfo.pushConstantRanges.data(),
  };
  VkPipelineLayout handle{VK_NULL_HANDLE};
  VK_CHECK(
    vkCreatePipelineLayout(m_logicalDevice, &createInfo, nullptr, &handle));

  const auto &[inserted, _] = m_pipelineLayouts.emplace(hash, handle);
  return PipelineLayout{inserted->second, std::move(descriptorSetLayouts)};
}

Texture RenderDevice::createTexture2D(const Extent2D extent,
                                      const PixelFormat format,
                                      const uint32_t numMipLevels,
                                      const uint32_t numLayers,
                                      const ImageUsage usageFlags) {
  assert(m_memoryAllocator != nullptr);
  return Texture{
    m_memoryAllocator,
    Texture::CreateInfo{
      .extent = extent,
      .depth = 0,
      .pixelFormat = format,
      .numMipLevels = numMipLevels,
      .numLayers = numLayers,
      .numFaces = 1,
      .usageFlags = usageFlags,
    },
  };
}
Texture RenderDevice::createTexture3D(const Extent2D extent,
                                      const uint32_t depth,
                                      const PixelFormat format,
                                      const uint32_t numMipLevels,
                                      const ImageUsage usageFlags) {
  assert(m_memoryAllocator != nullptr);
  return Texture{
    m_memoryAllocator,
    Texture::CreateInfo{
      .extent = extent,
      .depth = depth,
      .pixelFormat = format,
      .numMipLevels = numMipLevels,
      .numLayers = 0,
      .numFaces = 1,
      .usageFlags = usageFlags,
    },
  };
}

Texture RenderDevice::createCubemap(const uint32_t size,
                                    const PixelFormat format,
                                    const uint32_t numMipLevels,
                                    const uint32_t numLayers,
                                    const ImageUsage usageFlags) {
  assert(m_memoryAllocator != nullptr);
  return Texture{
    m_memoryAllocator,
    Texture::CreateInfo{
      .extent = {size, size},
      .depth = 0,
      .pixelFormat = format,
      .numMipLevels = numMipLevels,
      .numLayers = numLayers,
      .numFaces = 6,
      .usageFlags = usageFlags,
    },
  };
}

RenderDevice &RenderDevice::setupSampler(Texture &texture,
                                         SamplerInfo samplerInfo) {
  assert(texture && bool(texture.getUsageFlags() & ImageUsage::Sampled));

  const auto props = getFormatProperties(texture.getPixelFormat());
  if (!bool(props.optimalTilingFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    samplerInfo.minFilter = TexelFilter::Nearest;
    samplerInfo.magFilter = TexelFilter::Nearest;
    samplerInfo.mipmapMode = MipmapMode::Nearest;
  }
  texture.setSampler(getSampler(samplerInfo));
  return *this;
}
VkSampler RenderDevice::getSampler(const SamplerInfo &samplerInfo) {
  const auto hash = std::hash<SamplerInfo>{}(samplerInfo);
  auto it = m_samplers.find(hash);
  if (it == m_samplers.cend()) {
    it = m_samplers.emplace(hash, _createSampler(samplerInfo)).first;
    SPDLOG_TRACE("Created Sampler: {}", hash);
  }
  return it->second;
}

std::expected<SPIRV, std::string>
RenderDevice::compile(ShaderType shaderType,
                      const std::string_view code) const {
  return m_shaderCompiler.compile(shaderType, code);
}
ShaderModule RenderDevice::createShaderModule(const ShaderType shaderType,
                                              const std::string_view code,
                                              ShaderReflection *reflection) {
  if (auto spv = compile(shaderType, code); spv) {
    return createShaderModule(*spv, reflection);
  } else {
    SPDLOG_ERROR(spv.error());
    return {};
  }
}
ShaderModule RenderDevice::createShaderModule(SPIRV spv,
                                              ShaderReflection *reflection) {
  assert(m_logicalDevice != nullptr);

  ShaderModule shaderModule{m_logicalDevice, spv};
  if (reflection) reflection->accumulate(std::move(spv));
  return shaderModule;
}

ComputePipeline RenderDevice::createComputePipeline(
  const std::string_view code, std::optional<PipelineLayout> pipelineLayout) {
  auto reflection =
    pipelineLayout ? std::nullopt : std::make_optional<ShaderReflection>();

  const auto shaderModule = createShaderModule(
    ShaderType::Compute, code,
    reflection ? std::addressof(reflection.value()) : nullptr);
  if (!shaderModule) return {};

  if (reflection) pipelineLayout = reflectPipelineLayout(*this, *reflection);
  assert(*pipelineLayout);

  const VkComputePipelineCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .stage =
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = VkShaderModule{shaderModule},
        .pName = "main",
      },
    .layout = pipelineLayout->getHandle(),
  };

  VkPipeline computePipeline{VK_NULL_HANDLE};
  VK_CHECK(vkCreateComputePipelines(m_logicalDevice, m_pipelineCache, 1,
                                    &createInfo, nullptr, &computePipeline));

  return ComputePipeline{
    m_logicalDevice,
    std::move(pipelineLayout.value()),
    reflection ? reflection->localSize.value() : glm::uvec3{},
    computePipeline,
  };
}

RenderDevice &RenderDevice::upload(Buffer &buffer, const VkDeviceSize offset,
                                   const VkDeviceSize size, const void *data) {
  assert(buffer && data);
  assert(m_logicalDevice != VK_NULL_HANDLE);

  auto *mappedMemory = std::bit_cast<std::byte *>(buffer.map());
  std::memcpy(mappedMemory + offset, data, size);
  buffer.flush().unmap();
  return *this;
}

RenderDevice &RenderDevice::destroy(VkFence &fence) {
  assert(fence != VK_NULL_HANDLE);
  assert(m_logicalDevice != VK_NULL_HANDLE);

  vkDestroyFence(m_logicalDevice, fence, nullptr);
  fence = VK_NULL_HANDLE;
  return *this;
}
RenderDevice &RenderDevice::destroy(VkSemaphore &semaphore) {
  assert(semaphore != VK_NULL_HANDLE);
  assert(m_logicalDevice != VK_NULL_HANDLE);

  vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
  semaphore = VK_NULL_HANDLE;
  return *this;
}

CommandBuffer RenderDevice::createCommandBuffer() {
  assert(m_logicalDevice != VK_NULL_HANDLE);

  const VkCommandBufferAllocateInfo allocateInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = m_commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };

  VkCommandBuffer handle{VK_NULL_HANDLE};
  VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocateInfo, &handle));
  const auto tracy = TracyVkContext(m_physicalDevice.handle, m_logicalDevice,
                                    m_genericQueue, handle);
  return CommandBuffer{
    m_logicalDevice, m_commandPool, handle, tracy, createFence(),
  };
}

RenderDevice &
RenderDevice::execute(const std::function<void(CommandBuffer &)> &f) {
  auto cb = createCommandBuffer();
  cb.begin();
  {
    TRACY_GPU_ZONE(cb, "ExecuteCommandBuffer");
    std::invoke(f, cb);
  }
  return execute(cb);
}
RenderDevice &RenderDevice::execute(CommandBuffer &cb, const JobInfo &jobInfo) {
  cb.flushBarriers().end();
  assert(cb._invariant(CommandBuffer::State::Executable));

  const VkCommandBufferSubmitInfo commandBufferInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
    .commandBuffer = cb.m_handle,
  };
  const VkSemaphoreSubmitInfo waitSemaphoreInfo{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    .semaphore = jobInfo.wait,
    .stageMask = jobInfo.waitStage,
  };
  const VkSemaphoreSubmitInfo signalSemaphoreInfo{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    .semaphore = jobInfo.signal,
  };
  const VkSubmitInfo2 submitInfo{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

    .waitSemaphoreInfoCount = jobInfo.wait != VK_NULL_HANDLE ? 1u : 0u,
    .pWaitSemaphoreInfos = jobInfo.wait ? &waitSemaphoreInfo : nullptr,

    .commandBufferInfoCount = 1,
    .pCommandBufferInfos = &commandBufferInfo,

    .signalSemaphoreInfoCount = jobInfo.signal != VK_NULL_HANDLE ? 1u : 0u,
    .pSignalSemaphoreInfos = jobInfo.signal ? &signalSemaphoreInfo : nullptr,
  };
  VK_CHECK(vkQueueSubmit2(m_genericQueue, 1, &submitInfo, cb.m_fence));

  cb.m_state = CommandBuffer::State::Pending;
  return *this;
}

RenderDevice &RenderDevice::present(Swapchain &swapchain,
                                    const VkSemaphore wait) {
  ZoneScopedN("RHI::Present");

  assert(swapchain);
  assert(m_genericQueue != VK_NULL_HANDLE);

  const VkPresentInfoKHR presentInfo{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = wait != VK_NULL_HANDLE ? 1u : 0u,
    .pWaitSemaphores = &wait,
    .swapchainCount = 1,
    .pSwapchains = &swapchain.m_handle,
    .pImageIndices = &swapchain.m_currentImageIndex,
  };
  switch (vkQueuePresentKHR(m_genericQueue, &presentInfo)) {
  case VK_SUBOPTIMAL_KHR:
  case VK_ERROR_OUT_OF_DATE_KHR:
    swapchain.recreate();
    [[fallthrough]];
  case VK_SUCCESS:
    break;

  default:
    assert(false);
  }

  return *this;
}

RenderDevice &RenderDevice::pushGarbage(Buffer &buffer) {
  m_garbageCollector.push(buffer);
  return *this;
}
RenderDevice &RenderDevice::pushGarbage(Texture &texture) {
  m_garbageCollector.push(texture);
  return *this;
}
RenderDevice &RenderDevice::stepGarbage(const FrameIndex::ValueType threshold) {
  ZoneScopedN("RHI::CollectGarbage");
  m_garbageCollector.step(threshold);
  return *this;
}

RenderDevice &RenderDevice::wait(const VkFence fence) {
  assert(fence != VK_NULL_HANDLE);
  assert(m_logicalDevice != VK_NULL_HANDLE);
  VK_CHECK(vkWaitForFences(m_logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX));
  return reset(fence);
}
RenderDevice &RenderDevice::reset(const VkFence fence) {
  assert(fence != VK_NULL_HANDLE);
  assert(m_logicalDevice != VK_NULL_HANDLE);
  VK_CHECK(vkResetFences(m_logicalDevice, 1, &fence));
  return *this;
}

RenderDevice &RenderDevice::waitIdle() {
  assert(m_logicalDevice != VK_NULL_HANDLE);
  VK_CHECK(vkDeviceWaitIdle(m_logicalDevice));
  return *this;
}

//
// (private):
//

void RenderDevice::_createInstance() {
  gladLoaderLoadVulkan(nullptr, nullptr, nullptr);

  const VkApplicationInfo applicationInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = nullptr,
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "Supernova",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = _TARGET_VERSION,
  };

#if _USE_VALIDATION_LAYERS
  std::vector<const char *> validationLayerNames;
  validationLayerNames = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_synchronization2",
  };
  const auto supportedLayers = enumerateInstanceLayers();
  for (const auto *name : validationLayerNames) {
    if (!queryInstanceLayer(supportedLayers, name)) {
      throw std::runtime_error{
        std::format("VkInstance layer: '{}' is not supported", name)};
    }
  }
#endif

  const auto kExtensions = std::array {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
      VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#if defined(_DEBUG) || defined(RHI_USE_DEBUG_MARKER)
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
  };
  const auto supportedExtensions = enumerateInstanceExtensions();
  for (const auto *name : kExtensions) {
    if (!queryExtension(supportedExtensions, name)) {
      throw std::runtime_error{
        std::format("VkInstance extension: '{}' is not supported", name)};
    }
  }

  const VkInstanceCreateInfo instanceInfo {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo,
#if _USE_VALIDATION_LAYERS
    .enabledLayerCount = uint32_t(validationLayerNames.size()),
    .ppEnabledLayerNames = validationLayerNames.data(),
#endif
    .enabledExtensionCount = uint32_t(kExtensions.size()),
    .ppEnabledExtensionNames = kExtensions.data(),
  };
  VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_instance));
}
void RenderDevice::_selectPhysicalDevice(
  const PhysicalDeviceSelector &physicalDeviceSelector) {
  const auto physicalDevices = getPhysicalDevices(m_instance);
  if (physicalDeviceSelector) {
    m_physicalDevice = physicalDeviceSelector(physicalDevices);
  }
  if (m_physicalDevice.handle == VK_NULL_HANDLE) {
    m_physicalDevice = physicalDevices.front();
  }
  gladLoaderLoadVulkan(m_instance, m_physicalDevice.handle, nullptr);
}
void RenderDevice::_createLogicalDevice(uint32_t familyIndex) {
  assert(familyIndex != VK_QUEUE_FAMILY_IGNORED);

  static constexpr auto kMandatoryDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
  };
  const auto supportedDeviceExtensions =
    enumeratePhysicalDeviceExtensions(m_physicalDevice.handle);
  for (const auto *name : kMandatoryDeviceExtensions) {
    if (!queryExtension(supportedDeviceExtensions, name)) {
      throw std::runtime_error{
        std::format("VkDevice extension: '{}' is not supported", name)};
    }
  }

  std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  FeatureBuilder featureBuilder{m_physicalDevice.handle};
  if constexpr (kTargetVersion >= VK_API_VERSION_1_2) {
    auto &vk12 =
      featureBuilder.requestExtensionFeatures<VkPhysicalDeviceVulkan12Features>(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES);
    vk12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    vk12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    vk12.runtimeDescriptorArray = VK_TRUE;
  } else {
    auto &ext =
      featureBuilder
        .requestExtensionFeatures<VkPhysicalDeviceDescriptorIndexingFeatures>(
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT);
    ext.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    ext.descriptorBindingVariableDescriptorCount = VK_TRUE;
    ext.runtimeDescriptorArray = VK_TRUE;
  }
  if constexpr (kTargetVersion >= VK_API_VERSION_1_3) {
    auto &vk13 =
      featureBuilder.requestExtensionFeatures<VkPhysicalDeviceVulkan13Features>(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES);
    vk13.synchronization2 = VK_TRUE;
    vk13.dynamicRendering = VK_TRUE;
  } else {
    deviceExtensions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    featureBuilder
      .requestExtensionFeatures<VkPhysicalDeviceSynchronization2Features>(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES)
      .synchronization2 = VK_TRUE;

    deviceExtensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    featureBuilder
      .requestExtensionFeatures<VkPhysicalDeviceDynamicRenderingFeatures>(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES)
      .dynamicRendering = VK_TRUE;
  }

  if (queryExtension(supportedDeviceExtensions,
                     VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)) {
    deviceExtensions.emplace_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    featureBuilder
      .requestExtensionFeatures<VkPhysicalDeviceMemoryPriorityFeaturesEXT>(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT)
      .memoryPriority = VK_TRUE;

    // UNASSIGNED-BestPractices-CreateDevice-PageableDeviceLocalMemory
    if (queryExtension(supportedDeviceExtensions,
                       VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME)) {
      deviceExtensions.emplace_back(
        VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
      featureBuilder
        .requestExtensionFeatures<
          VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT>(
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT)
        .pageableDeviceLocalMemory = VK_TRUE;
    }
  }

  const auto kDefaultQueuePriority = 1.0f;
  const VkDeviceQueueCreateInfo queueCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = familyIndex,
    .queueCount = 1,
    .pQueuePriorities = &kDefaultQueuePriority,
  };

  // clang-format off
  const VkPhysicalDeviceFeatures enabledFeatures {
    .imageCubeArray = VK_TRUE, // Shadows.
    .independentBlend = VK_TRUE,
    .geometryShader = VK_TRUE,
    .depthClamp = VK_TRUE,
    .depthBiasClamp = VK_TRUE,
    .fillModeNonSolid = VK_TRUE, // Wireframe rendering.
    .wideLines = VK_TRUE,
    .samplerAnisotropy = VK_TRUE,
    //.textureCompressionETC2 = VK_TRUE,
    //.textureCompressionASTC_LDR = VK_TRUE,
    //.textureCompressionBC = VK_TRUE,
    .shaderTessellationAndGeometryPointSize = VK_TRUE,
  };
  // clang-format on

  const VkDeviceCreateInfo deviceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = featureBuilder.getRoot(),

    // .flags Reserved for future use.

    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queueCreateInfo,

    // Device layers are deprecated and ignored.

    .enabledExtensionCount = uint32_t(deviceExtensions.size()),
    .ppEnabledExtensionNames = deviceExtensions.data(),
    .pEnabledFeatures = &enabledFeatures,
  };

  const auto result = vkCreateDevice(m_physicalDevice.handle, &deviceCreateInfo,
                                     nullptr, &m_logicalDevice);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Could not create device."};
  }
  gladLoaderLoadVulkan(m_instance, m_physicalDevice.handle, m_logicalDevice);

  vkGetDeviceQueue(m_logicalDevice, familyIndex, 0, &m_genericQueue);
}
void RenderDevice::_createMemoryAllocator() {
#if _USE_VMA_LOGGER
  using namespace std::chrono_literals;

#  if 1
  auto duplicateFilter =
    std::make_unique<spdlog::sinks::dup_filter_sink_st>(3s);
  duplicateFilter->add_sink(
    std::make_unique<spdlog::sinks::stdout_color_sink_mt>());
  auto logger =
    std::make_shared<spdlog::logger>("VMA", std::move(duplicateFilter));
#  else
  auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("VMA", std::move(consoleSink));
#  endif
  logger->set_level(spdlog::level::trace);
  logger->set_pattern("[%n] %v");
  spdlog::register_logger(std::move(logger));
#endif

  const VmaAllocatorCreateInfo createInfo{
    .physicalDevice = m_physicalDevice.handle,
    .device = m_logicalDevice,
    .instance = m_instance,
    .vulkanApiVersion = kTargetVersion,
  };
  VK_CHECK(vmaCreateAllocator(&createInfo, &m_memoryAllocator));
}

VkDevice RenderDevice::_getLogicalDevice() const { return m_logicalDevice; }
VkQueue RenderDevice::_getGenericQueue() const { return m_genericQueue; }
VkPipelineCache RenderDevice::_getPipelineCache() const {
  return m_pipelineCache;
}

VkSampler RenderDevice::_createSampler(const SamplerInfo &i) {
  const VkSamplerCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

    .magFilter = VkFilter(i.magFilter),
    .minFilter = VkFilter(i.minFilter),
    .mipmapMode = VkSamplerMipmapMode(i.mipmapMode),

    .addressModeU = VkSamplerAddressMode(i.addressModeS),
    .addressModeV = VkSamplerAddressMode(i.addressModeT),
    .addressModeW = VkSamplerAddressMode(i.addressModeR),

    .mipLodBias = 0.0f,

    .anisotropyEnable = i.maxAnisotropy.has_value(),
    .maxAnisotropy = i.maxAnisotropy
                       ? glm::clamp(*i.maxAnisotropy, 1.0f,
                                    getDeviceLimits().maxSamplerAnisotropy)
                       : 0.0f,

    .compareEnable = i.compareOp.has_value(),
    .compareOp = VkCompareOp(i.compareOp.value_or(CompareOp::Less)),

    .minLod = i.minLod,
    .maxLod = i.maxLod,

    .borderColor = VkBorderColor(i.borderColor),
    .unnormalizedCoordinates = VK_FALSE,
  };
  VkSampler sampler{VK_NULL_HANDLE};
  VK_CHECK(vkCreateSampler(m_logicalDevice, &createInfo, nullptr, &sampler));
  return sampler;
}

//
// Helper:
//

PhysicalDevice
defaultDeviceSelector(std::span<const PhysicalDevice> physicalDevices) {
  const auto isDiscreteType = [](const PhysicalDevice &device) {
    return device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  };
  const auto it = std::ranges::find_if(physicalDevices, isDiscreteType);
  return it != physicalDevices.cend() ? *it : physicalDevices.front();
}

PhysicalDeviceSelector selectVendor(const Vendor vendor) {
  return [vendor](std::span<const PhysicalDevice> physicalDevices) {
    const auto isMadeBy = [vendor](const PhysicalDevice &device) {
      return device.properties.vendorID == std::to_underlying(vendor);
    };
    const auto it = std::ranges::find_if(physicalDevices, isMadeBy);
    return it != physicalDevices.cend() ? *it : PhysicalDevice{};
  };
}

} // namespace rhi
