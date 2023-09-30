---@meta

ui = {}

---Adds a new font face to the font engine. The face's family, style and weight will be determined from the face itself.
---@param filePath string # The path to the file to load the face from. The path is passed directly to the file interface which is used to load the file. The default file interface accepts both absolute paths and paths relative to the working directory.
---@return boolean # True if the face was loaded successfully, false otherwise.
function ui.loadFontFace(filePath) end

---Fetches a previously constructed context by name.
---@param name string # The name of the desired context.
---@return ui.Context # The desired context, or nullptr if no context exists with the given name.
function ui.getContext(name) end

---Returns the number of active contexts.
---@return integer # The total number of active RmlUi contexts.
function ui.getNumContexts() end

---@param context ui.Context
---@param evt InputEvent
function ui.processEvent(context, evt) end

---@param mousePos ivec2
---@param targetTextureExtent ivec2
---@param uiDimensions ivec2
---@return ivec2
function ui.scaleMousePosition(mousePos, targetTextureExtent, uiDimensions) end

---@class ui.FontFaceHandle
ui.FontFaceHandle = {}
