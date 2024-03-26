#pragma once

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"
#include "StorageBuffer.hpp"
#include "Texture.hpp"
#include "SamplerInfo.hpp"

#include "ComputePipeline.hpp"
#include "ShaderModule.hpp"
#include "ShaderCompiler.hpp"

#include "CommandBuffer.hpp"
#include "Swapchain.hpp"

#include "GarbageCollector.hpp"

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

  [[nodiscard]] VkFormatProperties getFormatProperties(const PixelFormat) const;

  // ---

  [[nodiscard]] Swapchain
  createSwapchain(const os::Window &,
                  const Swapchain::Format = Swapchain::Format::sRGB,
                  const VerticalSync = VerticalSync::Enabled);

  [[nodiscard]] VkFence createFence(const bool signaled = true);
  [[nodiscard]] VkSemaphore createSemaphore();

  // ---

  [[nodiscard]] Buffer createStagingBuffer(const VkDeviceSize size,
                                           const void *data = nullptr);

  [[nodiscard]] VertexBuffer
  createVertexBuffer(const Buffer::Stride, const VkDeviceSize capacity,
                     const AllocationHints = AllocationHints::None);
  [[nodiscard]] IndexBuffer
  createIndexBuffer(const IndexType, const VkDeviceSize capacity,
                    const AllocationHints = AllocationHints::None);
  [[nodiscard]] UniformBuffer
  createUniformBuffer(const VkDeviceSize size,
                      const AllocationHints = AllocationHints::None);
  [[nodiscard]] StorageBuffer
  createStorageBuffer(const VkDeviceSize size,
                      const AllocationHints = AllocationHints::None);

  [[nodiscard]] std::string getMemoryStats() const;

  // ---

  // .first = Hashed VkDescriptorSetLayout.
  [[nodiscard]] std::pair<std::size_t, VkDescriptorSetLayout>
  createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &);

  [[nodiscard]] PipelineLayout createPipelineLayout(const PipelineLayoutInfo &);

  [[nodiscard]] Texture createTexture2D(const Extent2D, const PixelFormat,
                                        const uint32_t numMipLevels,
                                        const uint32_t numLayers,
                                        const ImageUsage);
  [[nodiscard]] Texture createTexture3D(const Extent2D, const uint32_t depth,
                                        const PixelFormat,
                                        const uint32_t numMipLevels,
                                        const ImageUsage);
  [[nodiscard]] Texture createCubemap(const uint32_t size, const PixelFormat,
                                      const uint32_t numMipLevels,
                                      const uint32_t numLayers,
                                      const ImageUsage);

  RenderDevice &setupSampler(Texture &, SamplerInfo);
  [[nodiscard]] VkSampler getSampler(const SamplerInfo &);

  [[nodiscard]] ShaderCompiler::Result
  compile(const ShaderType, const std::string_view code) const;
  [[nodiscard]] ShaderModule createShaderModule(const ShaderType,
                                                const std::string_view code,
                                                ShaderReflection * = nullptr);
  [[nodiscard]] ShaderModule createShaderModule(SPIRV,
                                                ShaderReflection * = nullptr);

  [[nodiscard]] ComputePipeline
  createComputePipeline(const std::string_view code,
                        std::optional<PipelineLayout> = std::nullopt);

  // ---

  RenderDevice &upload(Buffer &, const VkDeviceSize offset,
                       const VkDeviceSize size, const void *data);

  RenderDevice &destroy(VkFence &);
  RenderDevice &destroy(VkSemaphore &);

  [[nodiscard]] CommandBuffer createCommandBuffer();

  // Blocking.
  RenderDevice &execute(const std::function<void(CommandBuffer &)> &);
  RenderDevice &execute(CommandBuffer &, const JobInfo & = {});

  // --

  RenderDevice &present(Swapchain &, const VkSemaphore wait = VK_NULL_HANDLE);

  // ---

  RenderDevice &pushGarbage(Buffer &);
  RenderDevice &pushGarbage(Texture &);
  RenderDevice &stepGarbage(const FrameIndex::ValueType threshold);

  // ---

  RenderDevice &wait(const VkFence);
  RenderDevice &reset(const VkFence);

  RenderDevice &waitIdle();

private:
  void _createInstance();
  void _selectPhysicalDevice(const PhysicalDeviceSelector &);
  void _createLogicalDevice(const uint32_t familyIndex);
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
[[nodiscard]] PhysicalDeviceSelector selectVendor(const Vendor);

template <class T, class... Args>
[[nodiscard]] auto makeShared(RenderDevice &rd, Args &&...args) {
  struct Deleter {
    RenderDevice &renderDevice;

    void operator()(T *resource) {
      renderDevice.pushGarbage(*resource);
      delete resource;
    }
  };
  return std::shared_ptr<T>{new T{std::forward<Args>(args)...}, Deleter{rd}};
}

} // namespace rhi

template <> struct has_flags<rhi::AllocationHints> : std::true_type {};
