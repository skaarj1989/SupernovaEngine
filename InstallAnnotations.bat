SET engine_annotations=\"%cd%\modules\Scripting\annotations\"
SET editor_annotations=\"%cd%\tools\Editor\annotations\"

SET settings_file=%APPDATA%\Code\User\settings.json
SET key=\"Lua.workspace.library\"

yq -iP ".[%key%] = (.[%key%] + [%engine_annotations%, %editor_annotations%] | unique)" %settings_file% -o json
