#!/bin/sh

engine_annotations=\"$PWD/tools/Editor/annotations\"
editor_annotations=\"$PWD/modules/Scripting/annotations\"

settings_file="$HOME/.config/Code/User/settings.json"
key=\"Lua.workspace.library\"

yq -iP ".[$key] = (.[$key] + [$engine_annotations, $editor_annotations] | unique)" $settings_file -o json
