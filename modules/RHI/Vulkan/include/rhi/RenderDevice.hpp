#pragma once

#include "rhi/ShaderCompiler.hpp"
#include "rhi/ShaderModule.hpp"
#include "rhi/SamplerInfo.hpp"
#include "rhi/UniformBuffer.hpp"
#include "rhi/StorageBuffer.hpp"
#include "rhi/CommandBuffer.hpp"
#include "rhi/Swapchain.hpp"
#include "rhi/GarbageCollector.hpp"
#include "rhi/DebugMarker.hpp"

namespace rhi {

struct PhysicalDevice {
  VkPhysicalDevice handle{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties properties{};
  VkPhysicalDeviceFeatures features{};
  VkPhysicalDeviceMemoryProperties memoryProperties{};
};
using PhysicalDeviceSelector =
  std::function<PhysicalDevice(std::span<const PhysicalDevice>)>;

struct PhysicalDeviceInfo {
  uint32_t vendorId;
  uint32_t deviceId;
  std::string_view deviceName;
};

struct JobInfo {
  VkSemaphore wait{VK_NULL_HANDLE};
  VkPipelineStageFlags waitStage{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
  VkSemaphore signal{VK_NULL_HANDLE};
};

enum class AllocationHints {
  None = 0,
  MinMemory = 1 << 0,
  SequentialWrite = 1 << 1,
};

class RenderDevice final {
  friend class GraphicsPipeline; // Uses m_logicalDevice and m_pipelineCache.

public:
  RenderDevice();
  explicit RenderDevice(const PhysicalDeviceSelector &);
  RenderDevice(const RenderDevice &) = delete;
  RenderDevice(RenderDevice &&) noexcept = default;
  ~RenderDevice();

  RenderDevice &operator=(const RenderDevice &) = delete;
  RenderDevice &operator=(RenderDevice &&) noexcept = default;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] std::string getName() const;

  [[nodiscard]] PhysicalDeviceInfo getPhysicalDeviceInfo() const;

  [[nodiscard]] const VkPhysicalDeviceLimits &getDeviceLimits() const;
  [[nodiscard]] const VkPhysicalDeviceFeatures &getDeviceFeatures() const;

  [[nodiscard]] VkFormatProperties getFormatProperties(PixelFormat) const;

  // ---

  [[nodiscard]] Swapchain
  createSwapchain(const os::Window &,
                  Swapchain::Format = Swapchain::Format::sRGB,
                  VerticalSync = VerticalSync::Enabled);

  [[nodiscard]] VkFence createFence(bool signaled = true);
  [[nodiscard]] VkSemaphore createSemaphore();

  // ---

  [[nodiscard]] Buffer createStagingBuffer(VkDeviceSize size,
                                           const void *data = nullptr);

  [[nodiscard]] VertexBuffer
  createVertexBuffer(uint32_t stride, VkDeviceSize capacity,
                     AllocationHints = AllocationHints::None);
  [[nodiscard]] IndexBuffer
  createIndexBuffer(IndexType, VkDeviceSize capacity,
                    AllocationHints = AllocationHints::None);
  [[nodiscard]] UniformBuffer
  createUniformBuffer(VkDeviceSize size,
                      AllocationHints = AllocationHints::None);
  [[nodiscard]] StorageBuffer
  createStorageBuffer(VkDeviceSize size,
                      AllocationHints = AllocationHints::None);

  [[nodiscard]] std::string getMemoryStats() const;

  // ---

  // .first = Hashed VkDescriptorSetLayout.
  [[nodiscard]] std::pair<std::size_t, VkDescriptorSetLayout>
  createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &);

  [[nodiscard]] PipelineLayout createPipelineLayout(const PipelineLayoutInfo &);

  [[nodiscard]] Texture createTexture2D(Extent2D, PixelFormat,
                                        uint32_t numMipLevels,
                                        uint32_t numLayers, ImageUsage);
  [[nodiscard]] Texture createTexture3D(Extent2D, uint32_t depth, PixelFormat,
                                        uint32_t numMipLevels, ImageUsage);
  [[nodiscard]] Texture createCubemap(uint32_t size, PixelFormat,
                                      uint32_t numMipLevels, uint32_t numLayers,
                                      ImageUsage);

  RenderDevice &setupSampler(Texture &, SamplerInfo);
  [[nodiscard]] VkSampler getSampler(const SamplerInfo &);

  [[nodiscard]] std::expected<SPIRV, std::string>
  compile(ShaderType, const std::string_view code) const;
  [[nodiscard]] ShaderModule createShaderModule(ShaderType,
                                                const std::string_view code,
                                                ShaderReflection * = nullptr);
  [[nodiscard]] ShaderModule createShaderModule(SPIRV,
                                                ShaderReflection * = nullptr);

  [[nodiscard]] ComputePipeline
  createComputePipeline(const std::string_view code,
                        std::optional<PipelineLayout> = std::nullopt);

  // ---

  RenderDevice &upload(Buffer &, VkDeviceSize offset, VkDeviceSize size,
                       const void *data);

  RenderDevice &destroy(VkFence &);
  RenderDevice &destroy(VkSemaphore &);

  [[nodiscard]] CommandBuffer createCommandBuffer();

  // Blocking.
  RenderDevice &execute(const std::function<void(CommandBuffer &)> &);
  RenderDevice &execute(CommandBuffer &, const JobInfo & = {});

  // --

  RenderDevice &present(Swapchain &, VkSemaphore wait = VK_NULL_HANDLE);

  // ---

  RenderDevice &pushGarbage(Buffer &);
  RenderDevice &pushGarbage(Texture &);
  RenderDevice &stepGarbage(const FrameIndex::ValueType threshold);

  // ---

  RenderDevice &wait(VkFence);
  RenderDevice &reset(VkFence);

  RenderDevice &waitIdle();

private:
  void _createInstance();
  void _selectPhysicalDevice(const PhysicalDeviceSelector &);
  void _createLogicalDevice(uint32_t familyIndex);
  void _createMemoryAllocator();

  [[nodiscard]] VkDevice _getLogicalDevice() const;
  [[nodiscard]] VkQueue _getGenericQueue() const;
  [[nodiscard]] VkPipelineCache _getPipelineCache() const;

  [[nodiscard]] VkSampler _createSampler(const SamplerInfo &);

private:
  VkInstance m_instance{VK_NULL_HANDLE};
  VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};

  PhysicalDevice m_physicalDevice;

  VkDevice m_logicalDevice{VK_NULL_HANDLE};
  VkQueue m_genericQueue{VK_NULL_HANDLE};
  VmaAllocator m_memoryAllocator;

  VkCommandPool m_commandPool{VK_NULL_HANDLE};
  VkPipelineCache m_pipelineCache{VK_NULL_HANDLE};

  template <typename T> using Cache = robin_hood::unordered_map<std::size_t, T>;

  Cache<VkSampler> m_samplers;

  Cache<VkDescriptorSetLayout> m_descriptorSetLayouts;
  Cache<VkPipelineLayout> m_pipelineLayouts;

  GarbageCollector m_garbageCollector;

  ShaderCompiler m_shaderCompiler;
};

//
// Helper:
//

[[nodiscard]] PhysicalDevice
  defaultDeviceSelector(std::span<const PhysicalDevice>);

enum class Vendor : uint32_t {
  AMD = 0x1002,
  NVIDIA = 0x10DE,
  Intel = 0x8086,
};
[[nodiscard]] PhysicalDeviceSelector selectVendor(Vendor);

template <class T, class... Args>
[[nodiscard]] auto makeShared(RenderDevice &rd, Args &&...args) {
  struct Deleter {
    rhi::RenderDevice &renderDevice;

    void operator()(T *resource) {
      renderDevice.pushGarbage(*resource);
      delete resource;
    }
  };
  return std::shared_ptr<T>{new T{std::forward<Args>(args)...}, Deleter{rd}};
}

} // namespace rhi

template <> struct has_flags<rhi::AllocationHints> : std::true_type {};

#define RHI_GPU_ZONE(CommandBuffer, Label)                                     \
  RHI_NAMED_DEBUG_MARKER(CommandBuffer, Label);                                \
  TRACY_GPU_TRANSIENT_ZONE(CommandBuffer, Label)
