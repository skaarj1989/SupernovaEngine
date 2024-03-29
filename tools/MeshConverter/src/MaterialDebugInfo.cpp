#include "MaterialDebugInfo.hpp"
#include "VisitorHelper.hpp"

#include "MakeSpan.hpp"

#include <variant>
#include <sstream>
#include <format>
#include <algorithm> // for_each
#include <cassert>

namespace {

using aiPropertyVariant =
  std::variant<int, float, aiString, aiColor3D, aiColor4D>;

[[nodiscard]] std::optional<aiPropertyVariant>
get(const aiMaterial &material, const aiMaterialProperty *p) {
#define GET(Type)                                                              \
  if (Type v; material.Get(p->mKey.C_Str(), p->mSemantic, p->mIndex, v) ==     \
              AI_SUCCESS) {                                                    \
    return v;                                                                  \
  }

  switch (p->mType) {
  case aiPTI_String:
    GET(aiString) break;

  case aiPTI_Float: {
#define CASE(Type)                                                             \
  case sizeof(Type):                                                           \
    GET(Type) break

    switch (p->mDataLength) {
      CASE(float);
      CASE(aiColor3D);
      CASE(aiColor4D);
    }

#undef CASE
  } break;

  case aiPTI_Integer:
    GET(int) break;
  }
#undef GET

  return std::nullopt;
}
[[nodiscard]] auto toString(const aiPropertyVariant &variant) {
  return std::visit(
    Overload{
      [](int v) { return std::format("int({})", v); },
      [](float v) { return std::format("float({})", v); },
      [](const aiString &v) { return std::format("'{}'", v.C_Str()); },
      [](const aiColor3D &v) {
        return std::format("aiColor3D({}, {}, {})", v.r, v.g, v.b);
      },
      [](const aiColor4D &v) {
        return std::format("aiColor4D({}, {}, {}, {})", v.r, v.g, v.b, v.a);
      },
    },
    variant);
}

[[nodiscard]] auto toString(aiPropertyTypeInfo type) {
#define CASE(Value)                                                            \
  case aiPTI_##Value:                                                          \
    return #Value

  switch (type) {
    CASE(Float);
    CASE(Double);
    CASE(String);
    CASE(Integer);
    CASE(Buffer);

  default:
    assert(false);
    return "Undefined";
  }
#undef CASE
}

} // namespace

[[nodiscard]] std::string toString(const aiMaterial &material) {
  std::ostringstream os;

  const auto addInfo = [&](const aiMaterialProperty *p) {
#if 0
    os << std::format("{}\t| {}: [{}, {}, {}]\n", toString(p->mType),
                      p->mKey.C_Str(),
                      aiTextureTypeToString(aiTextureType(p->mSemantic)),
                      p->mIndex, p->mDataLength);
#else
    const auto v = get(material, p);
    os << p->mKey.C_Str() << " = " << (v ? toString(*v) : "unknown") << "\n";
#endif
  };
  os << "Name: " << material.GetName().C_Str() << "\n";
  std::ranges::for_each(MAKE_SPAN(material, Properties), addInfo);

  return os.str();
}
