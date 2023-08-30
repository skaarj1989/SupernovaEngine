find_package(spdlog REQUIRED)

find_package(robin_hood REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)

add_subdirectory(extern)

find_file(GPU_MEM_DUMP_VIS "GpuMemDumpVis.py" "extern/VulkanMemoryAllocator/tools/GpuMemDumpVis")
configure_file("cmake/GpuMemDumpVis.bat.in" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/GpuMemDumpVis.bat)

add_library(VulkanRHI
  "src/VkCheck.hpp"

  "include/rhi/GarbageCollector.hpp"
  "src/GarbageCollector.cpp"

  "include/rhi/Offset2D.hpp"
  "include/rhi/Extent2D.hpp"
  "src/Extent2D.cpp"
  "include/rhi/Rect2D.hpp"
  "include/rhi/TexelFilter.hpp"
  "include/rhi/CompareOp.hpp"
  "include/rhi/SamplerInfo.hpp"
  "include/rhi/TextureType.hpp"
  "src/TextureType.cpp"
  "include/rhi/CubeFace.hpp"
  "src/CubeFace.cpp"
  "include/rhi/PrimitiveTopology.hpp"
  "src/PrimitiveTopology.cpp"
  "include/rhi/ShaderType.hpp"
  "src/ShaderType.cpp"
  "include/rhi/CullMode.hpp"
  "src/CullMode.cpp"
  "include/rhi/ImageLayout.hpp"
  "include/rhi/PipelineStage.hpp"
  "include/rhi/Access.hpp"
  "include/rhi/BarrierScope.hpp"
  "include/rhi/PixelFormat.hpp"
  "src/PixelFormat.cpp"
  "include/rhi/VertexAttributes.hpp"
  "src/VertexAttributes.cpp"

  # ---
  "include/rhi/Buffer.hpp"
  "src/Buffer.cpp"
  "include/rhi/VertexBuffer.hpp"
  "src/VertexBuffer.cpp"
  "include/rhi/IndexBuffer.hpp"
  "src/IndexBuffer.cpp"
  "include/rhi/UniformBuffer.hpp"
  "include/rhi/StorageBuffer.hpp"

  # ---
  "include/rhi/RenderDevice.hpp"
  "src/RenderDevice.cpp"
  "src/FeatureBuilder.hpp"
  "src/DebugCallback.hpp"
  "src/DebugCallback.cpp"
  "include/rhi/CommandBuffer.hpp"
  "src/CommandBuffer.cpp"
  "include/rhi/DebugMarker.hpp"
  "src/DebugMarker.cpp"
  "include/rhi/Barrier.hpp"
  "src/Barrier.cpp"
  "include/rhi/SPIRV.hpp"
  "include/rhi/ShaderCompiler.hpp"
  "src/ShaderCompiler.cpp"
  "src/ShaderReflection.hpp"
  "src/ShaderReflection.cpp"
  "include/rhi/DescriptorPool.hpp"
  "src/DescriptorPool.cpp"
  "include/rhi/DescriptorSetBuilder.hpp"
  "src/DescriptorSetBuilder.cpp"
  "include/rhi/FramebufferInfo.hpp"
  "src/FramebufferInfo.cpp"
  "include/rhi/GeometryInfo.hpp"

  # ---
  "include/rhi/PipelineLayout.hpp"
  "src/PipelineLayout.cpp"
  "include/rhi/ShaderModule.hpp"
  "src/ShaderModule.cpp"
  "include/rhi/BasePipeline.hpp"
  "src/BasePipeline.cpp"
  "include/rhi/ComputePipeline.hpp"
  "src/ComputePipeline.cpp"
  "include/rhi/GraphicsPipeline.hpp"
  "src/GraphicsPipeline.cpp"

  # ---
  "include/rhi/Swapchain.hpp"
  "src/Swapchain.cpp"
  "src/Win32Swapchain.cpp"

  # ---
  "include/rhi/Texture.hpp"
  "src/Texture.cpp"
  "include/rhi/TextureUtility.hpp"
  "src/TextureUtility.cpp"
  "include/rhi/RenderTargetView.hpp"

  # ---
  "include/rhi/FrameController.hpp"
  "src/FrameController.cpp"
  "include/rhi/FrameIndex.hpp"
  "src/FrameIndex.cpp"

  "include/rhi/BasePass.hpp"
  "include/rhi/RenderPass.hpp"
  "include/rhi/ComputePass.hpp"

  # ---
  "include/rhi/json.hpp"

  # ---
  "src/vk_mem_alloc.cpp"
)

target_compile_definitions(VulkanRHI PUBLIC VULKAN_RHI GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(VulkanRHI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(VulkanRHI
  PUBLIC
  Common
  Math
  Window
  glad::vk
  robin_hood::robin_hood
  VulkanMemoryAllocator
  PRIVATE
  StringUtility
  glslang::glslang
  glslang::OGLCompiler
  glslang::SPIRV
  spirv-cross-glsl
  spirv-cross-reflect
  spdlog::spdlog
)
set_target_properties(glad-vk VulkanMemoryAllocator VulkanRHI PROPERTIES FOLDER "Framework/RHI/Vulkan")

enable_profiler(VulkanRHI)

add_subdirectory(example)
