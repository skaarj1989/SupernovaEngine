#include "LuaWorldRenderer.hpp"
#include "TypeTraits.hpp"
#include "sol/state.hpp"

#include "renderer/Light.hpp"
#include "renderer/SkyLight.hpp"
#include "renderer/DecalInstance.hpp"
#include "renderer/RenderSettings.hpp"

#include "Sol2HelperMacros.hpp"

using namespace gfx;

namespace {

void registerLight(sol::state &lua) {
  // clang-format off
#define MAKE_PAIR(Key) _MAKE_PAIR(LightType, Key)
  lua.DEFINE_ENUM(LightType, {
    MAKE_PAIR(Directional),
    MAKE_PAIR(Spot),
    MAKE_PAIR(Point),
  });
#undef MAKE_PAIR

#define BIND(Member) _BIND(Light, Member)
	lua.DEFINE_USERTYPE(Light,
    sol::call_constructor,
    sol::factories(
      [] { return Light{}; },
      [](const sol::table &t) {
       return Light{
          CAPTURE_FIELD(type, LightType::Point),
          CAPTURE_FIELD(visible, true),
          CAPTURE_FIELD(castsShadow, true),
          CAPTURE_FIELD(debugVolume, false),
          CAPTURE_FIELD_T(color, glm::vec3, {1.0f}),
          CAPTURE_FIELD(intensity, 1.0f),
          CAPTURE_FIELD(range, 1.0f),
          CAPTURE_FIELD(innerConeAngle, 15.0f),
          CAPTURE_FIELD(outerConeAngle, 15.0f),
          CAPTURE_FIELD(shadowBias, 0.0f),
        };
      }
    ),

    BIND(type),
    BIND(visible),
    BIND(castsShadow),
    BIND(debugVolume),

    BIND(color),
    BIND(intensity),
    BIND(range),

    BIND(innerConeAngle),
    BIND(outerConeAngle),
    
    BIND(shadowBias),

    BIND_TYPEID(Light),
    BIND_TOSTRING(Light)
  );
#undef BIND
  // clang-format on
}

void registerSkyLight(sol::state &lua) {
  using gfx::SkyLight;

  // clang-format off
  lua.DEFINE_USERTYPE(SkyLight,
    sol::no_constructor,

    "source", sol::readonly_property([](const SkyLight &self) {
      return self.source.handle();
    }),

    BIND_TOSTRING(SkyLight)
  );
  // clang-format on
}

void registerResources(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(TextureResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource, rhi::Texture>(),

    BIND_TOSTRING(TextureResource)
  );
  lua["loadTexture"] = loadResource<TextureManager>;

  lua.DEFINE_USERTYPE(MaterialResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource, Material>(),

    BIND_TOSTRING(MaterialResource)
  );
  lua["loadMaterial"] = loadResource<MaterialManager>;

  lua.DEFINE_USERTYPE(MeshResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource, Mesh>(),

    BIND_TOSTRING(MeshResource)
  );
  lua["loadMesh"] = loadResource<MeshManager>;
  // clang-format on
}

template <typename T> auto forceType(const gfx::Property::Value &v) {
  return std::visit(
    [](const auto &arg) {
      using U = std::decay_t<decltype(arg)>;
      if constexpr (is_any_v<U, uint32_t, int32_t, float>) {
        return gfx::Property::Value{static_cast<T>(arg)};
      }
      return gfx::Property::Value{arg};
    },
    v);
}

void registerMaterialInstance(sol::state &lua) {
  enum class Numeric { Int, UInt, Float };
  // clang-format off
#define MAKE_PAIR(Key) _MAKE_PAIR(Numeric, Key)
  lua.DEFINE_ENUM(Numeric, {
    MAKE_PAIR(Int),
    MAKE_PAIR(UInt),
    MAKE_PAIR(Float),
  });
#undef MAKE_PAIR

#define BIND(Member) _BIND(MaterialInstance, Member)
  lua.DEFINE_USERTYPE(MaterialInstance,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<MaterialResource> resource) {
        return MaterialInstance{std::move(resource)};
      }
    ),

    "getResource",
      [](const MaterialInstance &self) {
        return std::dynamic_pointer_cast<MaterialResource>(self.getPrototype());
      },

    BIND(hasProperties),
    BIND(hasTextures),
        
    "setProperty", sol::overload(
      &MaterialInstance::setProperty,
      // Numeric type disambiguation.
      [](MaterialInstance &self, const std::string_view name,
         Property::Value v, Numeric type) -> MaterialInstance & {
        switch (type) {
          using enum Numeric;

        case Int: v = forceType<int32_t>(v); break;
        case UInt: v = forceType<uint32_t>(v); break;
        case Float: v = forceType<float>(v); break;
        }
        return self.setProperty(name, v);
      }
    ),
    BIND(getProperty),

    "setTexture",
      [](MaterialInstance &self, const std::string_view alias,
         std::shared_ptr<rhi::Texture> texture) -> MaterialInstance & {
        return self.setTexture(alias, std::move(texture));
      },

    "castsShadow", sol::property(
      &MaterialInstance::castsShadow,
      &MaterialInstance::setCastShadow
    ),
    "receivesShadow", sol::property(
      &MaterialInstance::receivesShadow,
      &MaterialInstance::setReceiveShadow
    ),
    "enabled", sol::property(
      &MaterialInstance::isEnabled,
      &MaterialInstance::enable
    ),

    BIND(reset),

    BIND_TOSTRING(MaterialInstance)
  );
#undef BIND
  // clang-format on
}

void registerMeshInstance(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(MeshInstance, Member)
	lua.DEFINE_USERTYPE(MeshInstance,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<MeshResource> meshResource) {
        return MeshInstance{std::move(meshResource)};
      }
    ),

    BIND(show),
    BIND(countVisible),

    "getResource",
      [](const MeshInstance &self) {
        return std::dynamic_pointer_cast<MeshResource>(self.getPrototype());
      },

    "setMaterial",
      [](MeshInstance &self, int32_t index,
         std::shared_ptr<MaterialResource> material) -> MeshInstance & {
        return self.setMaterial(index, std::move(material));
      },

    BIND(getMaterial),
    BIND(hasSkin),
    
    BIND(reset),

    BIND_TYPEID(MeshInstance),
    BIND_TOSTRING(MeshInstance)
  );
#undef BIND
  // clang-format on
}
void registerDecalInstance(sol::state &lua) {
  // clang-format off
	lua.DEFINE_USERTYPE(DecalInstance,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<MeshResource> resource) {
        return DecalInstance{std::move(resource)};
      }
    ),

    // WARNING:
    // The following line MUST BE EXACTLY HERE! (after sol::factories).
    // Do not move it!
    sol::base_classes, sol::bases<MeshInstance>(),

    BIND_TYPEID(DecalInstance),
    BIND_TOSTRING(DecalInstance)
  );
  // clang-format on
}

void registerRenderSettings(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(RenderSettings, Member)
  lua.DEFINE_USERTYPE(RenderSettings,
    BIND(outputMode),
    BIND(features),

    BIND(ambientLight),
    BIND(IBLIntensity),
    BIND(globalIllumination),
    BIND(shadow),
    
    BIND(ssao),
    BIND(bloom),
    BIND(exposure),
    BIND(adaptiveExposure),
    BIND(tonemap),
    BIND(debugFlags),

    BIND_TOSTRING(RenderSettings)
  );
#undef BIND

#define MAKE_PAIR(Key) _MAKE_PAIR(OutputMode, Key)
  lua.DEFINE_ENUM(OutputMode, {
    MAKE_PAIR(Depth),
    MAKE_PAIR(Normal),
    MAKE_PAIR(Emissive),
    MAKE_PAIR(BaseColor),
    MAKE_PAIR(Metallic),
    MAKE_PAIR(Roughness),
    MAKE_PAIR(AmbientOcclusion),

    MAKE_PAIR(SSAO),
    MAKE_PAIR(BrightColor),
    MAKE_PAIR(Reflections),

    MAKE_PAIR(Accum),
    MAKE_PAIR(Reveal),

    MAKE_PAIR(LightHeatmap),

    MAKE_PAIR(HDR),
    MAKE_PAIR(FinalImage),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Key) _MAKE_PAIR(RenderFeatures, Key)
  lua.DEFINE_ENUM(RenderFeatures, {
    MAKE_PAIR(None),

    MAKE_PAIR(LightCulling),
    MAKE_PAIR(SoftShadows),
    MAKE_PAIR(GI),
    MAKE_PAIR(SSAO),
    MAKE_PAIR(SSR),
    MAKE_PAIR(Bloom),
    MAKE_PAIR(FXAA),
    MAKE_PAIR(EyeAdaptation),
    MAKE_PAIR(CustomPostprocess),

    MAKE_PAIR(Default),

    MAKE_PAIR(All),
  });
#undef MAKE_PAIR

#define BIND(Member) _BIND(RenderSettings::GlobalIllumination, Member)
  lua DEFINE_NESTED_USERTYPE(RenderSettings, GlobalIllumination,
    BIND(numPropagations),
    BIND(intensity)
  );
#undef BIND

#define BIND(Member) _BIND(RenderSettings::Bloom, Member)
  lua DEFINE_NESTED_USERTYPE(RenderSettings, Bloom,
    BIND(radius),
    BIND(strength)
  );
#undef BIND

#define MAKE_PAIR(Key) _MAKE_PAIR(Tonemap, Key)
  lua.DEFINE_ENUM(Tonemap, {
    MAKE_PAIR(Clamp),
    MAKE_PAIR(ACES),
    MAKE_PAIR(Filmic),
    MAKE_PAIR(Reinhard),
    MAKE_PAIR(Uncharted),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Key) _MAKE_PAIR(DebugFlags, Key)
  lua.DEFINE_ENUM(DebugFlags, {
    MAKE_PAIR(None),

    MAKE_PAIR(WorldBounds),
    MAKE_PAIR(InfiniteGrid),

    MAKE_PAIR(Wireframe),
    MAKE_PAIR(VertexNormal),

    MAKE_PAIR(CascadeSplits),
    MAKE_PAIR(LightHeatmap),

    MAKE_PAIR(VPL),
    MAKE_PAIR(IrradianceOnly),
  });
#undef MAKE_PAIR
  // clang-format on
}

} // namespace

void registerWorldRenderer(sol::state &lua) {
  registerLight(lua);
  registerSkyLight(lua);

  registerResources(lua);
  registerMaterialInstance(lua);
  registerMeshInstance(lua);
  registerDecalInstance(lua);

  registerRenderSettings(lua);
}
