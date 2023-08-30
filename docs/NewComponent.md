# New Component

```cpp
struct ComponentType {
    // ...

    template <class Archive> void serialize(Archive &archive) { archive(...); }
};
static_assert(std::is_copy_constructible_v<ComponentType>);
```

## Scene

### Automatic serialization

Add the **ComponentType** to:
`Scene::kCoreComponentTypes`

### Scripting

`./modules/Scripting/modules/Components`

```cmake
# CMakeLists.txt

define_script_module(
  TARGET ComponentType
  SOURCES "include/LuaComponentType.hpp" "src/LuaComponentType.cpp"
)
```

```cpp
// LuaComponentType.cpp

void registerComponentType(sol::state &lua) {
  lua.new_usertype<ComponentType>("ComponentType",
    sol::no_constructor,

    // member variables, functions ...

    "type_id", &entt::type_hash<ComponentType>::value,
    sol::meta_function::to_string, [](const ComponentType &self) {
      return "ComponentType";
    }
  );
}
```

#### Automatic module inclusion and registration

`./modules/Scripting/include/LuaModules.hpp`

### Editor

#### GUI

Add menu entry
`SceneEditor::_inspectorWidget`

#### Inspector

Declare and implement an inspector function

```cpp
// SceneEditor/Inspectors/ComponentType.cpp

#include "SceneEditor/SceneEditor.hpp"

SceneEditor::_onInspect(entt::handle, ComponentType &)
```

```cpp
// SceneEditor::SceneEditor()

// Without the following call, you'll see an empty inspector and a 'Remove' button.
_connectInspector<ComponentType>(); 
```
