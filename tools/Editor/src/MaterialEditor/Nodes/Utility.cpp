#include "Utility.hpp"
#include "AlwaysFalse.hpp"

#include "MaterialEditor/Nodes/Empty.hpp"
#include "MaterialEditor/ValueVariant.hpp"
#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"

NodeBase *createNode(ShaderGraph &g, std::optional<VertexID> hint,
                     const TransientVariant &variant) {
  return std::visit(
    [&g, hint](const auto &arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (std::is_empty_v<T>) {
        return g.add<EmptyNode>(hint);
      } else if constexpr (std::is_same_v<T, ValueVariant>) {
        return g.add<EmbeddedNode<ValueVariant>>(hint, arg);
      } else if constexpr (std::is_scoped_enum_v<T>) {
        return g.add<EmbeddedNode<T>>(hint, arg);
      } else if constexpr (std::is_same_v<T, TextureParam>) {
        return g.add<EmbeddedNode<T>>(hint, arg);
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    variant);
}
