#pragma once

#if __has_include("LuaUtility.hpp")
#  include "LuaUtility.hpp"
#endif
#if __has_include("LuaLogger.hpp")
#  include "LuaLogger.hpp"

#endif

#if __has_include("LuaMath.hpp")
#  include "LuaMath.hpp"
#endif

#if __has_include("LuaFileSystem.hpp")
#  include "LuaFileSystem.hpp"
#endif
#if __has_include("LuaWindow.hpp")
#  include "LuaWindow.hpp"
#endif
#if __has_include("LuaInputSystem.hpp")
#  include "LuaInputSystem.hpp"
#endif

#if __has_include("LuaResource.hpp")
#  include "LuaResource.hpp"
#endif

#if __has_include("LuaScheduler.hpp")
#  include "LuaScheduler.hpp"
#endif
#if __has_include("LuaDispatcher.hpp")
#  include "LuaDispatcher.hpp"
#endif

#if __has_include("LuaEntity.hpp")
#  include "LuaEntity.hpp"
#endif

#if __has_include("LuaTransform.hpp")
#  include "LuaTransform.hpp"
#endif
#if __has_include("LuaCamera.hpp")
#  include "LuaCamera.hpp"
#endif

#if __has_include("LuaDebugDraw.hpp")
#  include "LuaDebugDraw.hpp"
#endif

#if __has_include("LuaPhysics.hpp")
#  include "LuaPhysics.hpp"
#endif

#if __has_include("LuaAnimation.hpp")
#  include "LuaAnimation.hpp"
#endif

#if __has_include("LuaVulkanRHI.hpp")
#  include "LuaVulkanRHI.hpp"
#endif
#if __has_include("LuaWorldRenderer.hpp")
#  include "LuaWorldRenderer.hpp"
#endif
#if __has_include("LuaCameraComponent.hpp")
#  include "LuaCameraComponent.hpp"
#endif

#if __has_include("LuaNameComponent.hpp")
#  include "LuaNameComponent.hpp"
#endif
#if __has_include("LuaScriptComponent.hpp")
#  include "LuaScriptComponent.hpp"
#endif

#if __has_include("LuaRmlUi.hpp")
#  include "LuaRmlUi.hpp"
#endif

inline void registerModules(sol::state &lua) {
#if __has_include("LuaUtility.hpp")
  registerUtility(lua);
#endif
#if __has_include("LuaLogger.hpp")
  registerLogger(lua);
#endif

#if __has_include("LuaMath.hpp")
  registerMath(lua);
#endif

#if __has_include("LuaFileSystem.hpp")
  registerFileSystem(lua);
#endif
#if __has_include("LuaWindow.hpp")
  registerWindow(lua);
#endif
#if __has_include("LuaInputSystem.hpp")
  registerInputSystem(lua);
#endif

#if __has_include("LuaResource.hpp")
  registerResource(lua);
#endif

#if __has_include("LuaScheduler.hpp")
  registerScheduler(lua);
#endif
#if __has_include("LuaDispatcher.hpp")
  registerDispatcher(lua);
#endif

#if __has_include("LuaEntity.hpp")
  registerEntityHandle(lua);
#endif

#if __has_include("LuaTransform.hpp")
  registerTransform(lua);
#endif
#if __has_include("LuaCamera.hpp")
  registerCamera(lua);
#endif

#if __has_include("LuaDebugDraw.hpp")
  registerDebugDraw(lua);
#endif

#if __has_include("LuaPhysics.hpp")
  registerJoltPhysics(lua);
#endif

#if __has_include("LuaAnimation.hpp")
  registerAnimation(lua);
#endif

#if __has_include("LuaVulkanRHI.hpp")
  registerVulkanRHI(lua);
#endif
#if __has_include("LuaWorldRenderer.hpp")
  registerWorldRenderer(lua);
#endif
#if __has_include("LuaCameraComponent.hpp")
  registerCameraComponent(lua);
#endif

#if __has_include("LuaNameComponent.hpp")
  registerNameComponent(lua);
#endif
#if __has_include("LuaScriptComponent.hpp")
  registerScriptComponent(lua);
#endif

#if __has_include("LuaRmlUi.hpp")
  registerRmlUi(lua);
#endif
}
