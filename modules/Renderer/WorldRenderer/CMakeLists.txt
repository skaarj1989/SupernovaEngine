find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

FetchContent_Declare(FrameGraph
  GIT_REPOSITORY https://github.com/skaarj1989/FrameGraph.git
  GIT_TAG master
)
set(FG_BUILD_TEST OFF CACHE BOOL "")
FetchContent_MakeAvailable(FrameGraph)

set_target_properties(FrameGraph PROPERTIES FOLDER "Framework/Renderer")

set(FG_DATA_SOURCES

  # -- PUBLIC:
  "include/renderer/FrameGraphData/LightPropagationVolumes.hpp"
  "include/renderer/FrameGraphData/ReflectiveShadowMap.hpp"

  # -- PRIVATE:
  "src/FrameGraphData/AverageLuminance.hpp"
  "src/FrameGraphData/BRDF.hpp"
  "src/FrameGraphData/BrightColor.hpp"
  "src/FrameGraphData/Camera.hpp"
  "src/FrameGraphData/DummyResources.hpp"
  "src/FrameGraphData/Frame.hpp"
  "src/FrameGraphData/GBuffer.hpp"
  "src/FrameGraphData/GlobalIllumination.hpp"
  "src/FrameGraphData/LightCulling.hpp"
  "src/FrameGraphData/Lights.hpp"
  "src/FrameGraphData/MaterialProperties.hpp"
  "src/FrameGraphData/Reflections.hpp"
  "src/FrameGraphData/SceneColor.hpp"
  "src/FrameGraphData/ShadowMap.hpp"
  "src/FrameGraphData/Skins.hpp"
  "src/FrameGraphData/SkyLight.hpp"
  "src/FrameGraphData/SSAO.hpp"
  "src/FrameGraphData/Transforms.hpp"
  "src/FrameGraphData/WeightedBlended.hpp"
  "src/FrameGraphData/UserData.hpp"
  "src/FrameGraphData/Silhouette.hpp"
)
set(FG_SOURCES

  # -- PUBLIC:
  "include/renderer/TransientResources.hpp"
  "src/TransientResources.cpp"

  "include/renderer/FrameGraphTexture.hpp"
  "src/FrameGraphTexture.cpp"
  "include/renderer/FrameGraphBuffer.hpp"
  "src/FrameGraphBuffer.cpp"

  # -- PRIVATE:
  "src/FrameGraphImport.hpp"
  "src/FrameGraphImport.cpp"

  "src/FrameGraphResourceAccess.hpp"
  "src/FrameGraphResourceAccess.cpp"

  "src/FrameGraphCommon.hpp"
  "src/FrameGraphCommon.cpp"
  "src/FrameGraphForwardPass.hpp"
  "src/FrameGraphForwardPass.cpp"
  "src/PipelineStage.hpp"
  "src/GBufferFlags.hpp"

  ${FG_DATA_SOURCES}
)

set(PASS_SOURCES
  "include/renderer/Technique.hpp"

  "src/BaseGeometryPassInfo.hpp"
  "src/BaseGeometryPassInfo.cpp"
  "src/ForwardPassInfo.hpp"
  "src/ForwardPassInfo.cpp"
  "src/LightingPassFeatures.hpp"
  "src/LightingPassFeatures.cpp"
  "src/LightingSettings.hpp"

  "include/renderer/Blit.hpp"
  "src/Blit.cpp"
  "include/renderer/Blur.hpp"
  "src/Blur.cpp"

  "include/renderer/Bloom.hpp"
  "src/Bloom.cpp"
  "include/renderer/Downsampler.hpp"
  "src/Downsampler.cpp"
  "include/renderer/Upsampler.hpp"
  "src/Upsampler.cpp"

  "include/renderer/GBufferPass.hpp"
  "src/GBufferPass.cpp"
  "include/renderer/DecalPass.hpp"
  "src/DecalPass.cpp"
  "include/renderer/DeferredLightingPass.hpp"
  "src/DeferredLightingPass.cpp"
  "include/renderer/SkyboxPass.hpp"
  "src/SkyboxPass.cpp"
  "include/renderer/TransparencyPass.hpp"
  "src/TransparencyPass.cpp"
  "include/renderer/TransmissionPass.hpp"
  "src/TransmissionPass.cpp"
  "include/renderer/WeightedBlendedPass.hpp"
  "src/WeightedBlendedPass.cpp"
  "include/renderer/TransparencyCompositionPass.hpp"
  "src/TransparencyCompositionPass.cpp"

  "include/renderer/SSAO.hpp"
  "src/SSAO.cpp"
  "include/renderer/SSR.hpp"
  "src/SSR.cpp"

  "include/renderer/AdaptiveExposure.hpp"
  "include/renderer/EyeAdaptation.hpp"
  "src/EyeAdaptation.cpp"

  "include/renderer/TonemapPass.hpp"
  "src/TonemapPass.cpp"
  "include/renderer/FXAA.hpp"
  "src/FXAA.cpp"
  "include/renderer/WireframePass.hpp"
  "src/WireframePass.cpp"
  "include/renderer/FinalPass.hpp"
  "src/FinalPass.cpp"
  "include/renderer/PostProcessor.hpp"
  "src/PostProcessor.cpp"
  "include/renderer/TiledLighting.hpp"
  "src/TiledLighting.cpp"
  "include/renderer/ShadowRenderer.hpp"
  "src/ShadowRenderer.cpp"

  "include/renderer/Grid.hpp"
  "include/renderer/GlobalIllumination.hpp"
  "src/GlobalIllumination.cpp"

  "include/renderer/DebugDrawPass.hpp"
  "src/DebugDrawPass.cpp"
  "include/renderer/DebugNormalPass.hpp"
  "src/DebugNormalPass.cpp"
  "include/renderer/InfiniteGridPass.hpp"
  "src/InfiniteGridPass.cpp"
  "include/renderer/OutlineRenderer.hpp"
  "src/OutlineRenderer.cpp"

  "src/TransientBuffer.hpp"
  "src/UploadStruct.hpp"
  "src/UploadContainer.hpp"

  "src/UploadFrameBlock.hpp"
  "src/UploadFrameBlock.cpp"
  "src/UploadCameraBlock.hpp"
  "src/UploadCameraBlock.cpp"
  "src/UploadTransforms.hpp"
  "src/UploadTransforms.cpp"
  "src/UploadSkins.hpp"
  "src/UploadSkins.cpp"
  "src/UploadMaterialProperties.hpp"
  "src/UploadMaterialProperties.cpp"
  "src/UploadInstances.hpp"
  "src/UploadInstances.cpp"
  "src/UploadLights.hpp"
  "src/UploadLights.cpp"
  "src/UploadShadowBlock.hpp"
  "src/UploadShadowBlock.cpp"
  "src/UploadSceneGrid.hpp"
  "src/UploadSceneGrid.cpp"
)

set(RESOURCE_SOURCES
  "include/renderer/MeshResource.hpp"
  "src/MeshResource.cpp"
  "include/renderer/MeshResourceHandle.hpp"
  "include/renderer/MeshLoader.hpp"
  "src/MeshLoader.cpp"
  "include/renderer/MeshManager.hpp"
  "src/MeshManager.cpp"

  "include/renderer/MaterialResource.hpp"
  "src/MaterialResource.cpp"
  "include/renderer/MaterialResourceHandle.hpp"
  "include/renderer/MaterialLoader.hpp"
  "src/MaterialLoader.cpp"
  "include/renderer/MaterialManager.hpp"
  "src/MaterialManager.cpp"

  "include/renderer/TextureResource.hpp"
  "src/TextureResource.cpp"
  "include/renderer/TextureResourceHandle.hpp"
  "include/renderer/TextureLoader.hpp"
  "src/TextureLoader.cpp"
  "include/renderer/TextureManager.hpp"
  "src/TextureManager.cpp"
)

set(SOURCES
  "include/renderer/VertexFormat.hpp"
  "src/VertexFormat.cpp"
  "include/renderer/jsonVertexFormat.hpp"

  "include/renderer/Vertex1p1n1st.hpp"
  "src/Vertex1p1n1st.cpp"

  "include/renderer/Mesh.hpp"
  "src/Mesh.cpp"

  "include/renderer/MaterialProperty.hpp"
  "src/MaterialProperty.cpp"
  "include/renderer/Material.hpp"
  "src/Material.cpp"
  "include/renderer/jsonMaterial.hpp"

  "include/renderer/TextureResources.hpp"
  "src/TextureResources.cpp"

  ${RESOURCE_SOURCES}

  "include/renderer/MeshInstance.hpp"
  "src/MeshInstance.cpp"
  "include/renderer/DecalInstance.hpp"
  "src/DecalInstance.cpp"

  "include/renderer/MaterialInstance.hpp"
  "src/MaterialInstance.cpp"

  "include/renderer/Light.hpp"
  "src/Light.cpp"

  "include/renderer/Renderable.hpp"
  "src/Renderable.cpp"

  "include/renderer/ViewInfo.hpp"

  "src/BasicShapes.hpp"
  "include/renderer/CubemapConverter.hpp"
  "src/CubemapConverter.cpp"
  "include/renderer/IBL.hpp"
  "src/IBL.cpp"
  "include/renderer/PostProcess.hpp"
  "src/PostProcess.cpp"

  "include/renderer/DummyResources.hpp"
  "src/DummyResources.cpp"

  "src/RenderContext.hpp"
  "src/RenderContext.cpp"

  "src/BuildPropertyBuffer.hpp"
  "src/BuildPropertyBuffer.cpp"
  "src/MaterialShader.hpp"
  "src/MaterialShader.cpp"
  "include/renderer/CodePair.hpp"

  "include/renderer/RawCamera.hpp"
  "include/renderer/Cascade.hpp"
  "src/BiasMatrix.hpp"
  "src/BiasMatrix.cpp"
  "src/Batch.hpp"
  "src/BatchBuilder.hpp"
  "src/BatchBuilder.cpp"
  "src/ShadowCascadesBuilder.hpp"
  "src/ShadowCascadesBuilder.cpp"

  "src/GPUInstance.hpp"

  "include/renderer/SceneView.hpp"
  "include/renderer/WorldView.hpp"

  "include/renderer/SkyLight.hpp"
  "src/SkyLight.cpp"
  "include/renderer/RenderSettings.hpp"
  "include/renderer/RenderFeatures.hpp"
  "src/RenderFeatures.cpp"
  "include/renderer/DebugFlags.hpp"
  "src/DebugFlags.cpp"
  "include/renderer/PipelineGroups.hpp"
  "include/renderer/OutputMode.hpp"
  "include/renderer/Tonemap.hpp"
  "include/renderer/ShadowSettings.hpp"
  "include/renderer/SSAOSettings.hpp"

  "include/renderer/CommonSamplers.hpp"

  "include/renderer/WorldRenderer.hpp"
  "src/WorldRenderer.cpp"

  ${PASS_SOURCES}
  ${FG_SOURCES}
)

add_library(WorldRenderer ${SOURCES})
target_compile_definitions(WorldRenderer
  PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE
)
target_include_directories(WorldRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(WorldRenderer
  PRIVATE
  spdlog::spdlog
  StringUtility
  ShaderCodeBuilder
  STBImageLoader
  KTXLoader
  nlohmann_json::nlohmann_json
  PUBLIC
  Resource
  Camera
  DebugDraw
  VulkanRHI
  fg::FrameGraph
)
set_target_properties(WorldRenderer PROPERTIES FOLDER "Framework/Renderer")
set_warning_level(WorldRenderer)

group_msvc_project(TARGET WorldRenderer)

add_resources(
  TARGET WorldRendererShaders
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  OUT_DIR shaders
)
add_dependencies(WorldRenderer Copy-WorldRendererShaders)
