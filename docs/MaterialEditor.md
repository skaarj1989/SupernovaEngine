# Material Editor

## Implementing custom scripted function

1. Create lua script in `assets/nodes/custom/`

```lua
-- Template

return FunctionInfo({
    category = "...",
    name = "foo", -- function name must be known to GLSL
    description = "...",
    signature = "float foo(float)",
    args = {
        Parameter({
            name = "x",
            description = "value",
            defaultValue = 0.0,
        }),
        -- more parameters if necessary ...
    },
    getReturnType = function(args)
        local x = args[1]
        return x == DataType.Float and DataType.Float or DataType.Undefined
    end,
})
```

2. Create GLSL file with function(s) in `modules/Renderer/WorldRender/shaders/`
3. Add include directive to `UserCodeCommons.glsl`

## Implementing custom node

_If a scripted function is not enough ..._

1. Create header/source pair in:

   - `include/MaterialEditor/Nodes/`
   - `src/MaterialEditor/Nodes/`

2. Extend `CompoundNodeVariant` located in `include/MaterialEditor/CompoundNodeVariant.hpp`

3. Add a menu entry in `MaterialEditor::_buildMenuEntries`

   ```cpp
   entries.push_back({.name = "Foo", .payload = FooNode::create});
   ```

## Template

### Header

```cpp
#pragma once

#include "NodeCommon.hpp"

struct FooNode : NodeBase {
    VertexDescriptor x;

    static FooNode create(ShaderGraph &, VertexDescriptor parent);
    FooNode clone(ShaderGraph &, VertexDescriptor) const;

    void remove(ShaderGraph &);

    bool inspect(ShaderGraph &, int32_t id);
    [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id) const;

    template <class Archive> void serialize(Archive &archive) {
        archive(Serializer{x});
    }
};
```

### Source

```cpp
#include "MaterialEditor/Nodes/Foo.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include "ImGuiHelper.hpp"

FooNode FooNode::create(ShaderGraph &graph, VertexDescriptor parent) {
    return {
        .x = createInternalInput(graph, parent, "x", ValueVariant{0.0f}),
    };
}
FooNode FooNode::clone(ShaderGraph &graph, VertexDescriptor parent) const {
    return create(graph, parent);
}

void FooNode::remove(ShaderGraph &graph) { graph.removeVertex(x); }

NodeResult FooNode::evaluate(MaterialGenerationContext &context, int32_t id) const {
    auto &[_, tokens, composer] = *context.currentShader;

    // const auto arg = extractTop(tokens);
    // const auto [aArg, bArg] = extract<2>(tokens);

    // ...
}
bool FooNode::inspect(ShaderGraph &graph, int32_t id) {
    constexpr auto kLabel = "Foo";
    constexpr auto kMinNodeWidth = 80.0f;
    const auto nodeWidth = glm::max(ImGui::CalcTextSize(kLabel).x, kMinNodeWidth);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(kLabel);
    ImNodes::EndNodeTitleBar();

    auto maxWidth = 0.0f;

    auto changed = false;

    changed | addInputPin(graph, x, {.name = "x"}, InspectorMode::Inline);
    maxWidth = glm::max(maxWidth, ImGui::GetItemRectSize().x);

    ImGui::Spacing();
    ImNodes::AddOutputAttribute(id, {.name = "out", .nodeWidth = glm::max(nodeWidth, maxWidth)});

    return changed;
}
```
