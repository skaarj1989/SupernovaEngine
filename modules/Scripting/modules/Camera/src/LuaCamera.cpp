#include "LuaCamera.hpp"
#include "sol/state.hpp"

#include "PerspectiveCamera.hpp"
#include "Transform.hpp"
#include "Sol2HelperMacros.hpp"

#include <format>

using namespace gfx;

void registerCamera(sol::state &lua) {
#define BIND(Member) _BIND(ClippingPlanes, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(ClippingPlanes, 
    sol::call_constructor,
    sol::constructors<ClippingPlanes(float, float)>(),

    BIND(zNear),
    BIND(zFar),

    sol::meta_function::to_string,
      [](const ClippingPlanes &self) {
        return std::format("ClippingPlanes(zNear={}, zFar={})",
                            self.zNear, self.zFar);
      }
  );
#undef BIND

#define BIND(Member) _BIND(PerspectiveCamera, Member)
  lua.DEFINE_USERTYPE(PerspectiveCamera,
    sol::call_constructor,
    sol::constructors<PerspectiveCamera()>(),
    
    BIND(setPerspective),
    
    BIND(setFov),
    BIND(setAspectRatio),
    BIND(setClippingPlanes),
    
    BIND(setPosition),
    BIND(setOrientation),
    BIND(fromTransform),

    BIND(getFov),
    BIND(getAspectRatio),
    BIND(getClippingPlanes),

    BIND(getPosition),
    BIND(getOrientation),

    BIND(getPitch),
    BIND(getYaw),
    BIND(getRoll),

    BIND(getRight),
    BIND(getUp),
    BIND(getForward),

    BIND(getView),
    BIND(getProjection),
    BIND(getViewProjection),

    BIND(getFrustum),

    BIND_TOSTRING(PerspectiveCamera)
  );
  // clang-format on
#undef BIND
}
