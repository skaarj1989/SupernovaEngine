---@meta

---@class ui.Debugger
ui.Debugger = {}

---Initialises the debug plugin. The debugger will be loaded into the given context.
---@param hostContext ui.Context # RmlUi context to load the debugger into. The debugging tools will be displayed on this context. If this context is destroyed, the debugger will be released.
---@return boolean # True if the debugger was successfully initialised
function ui.Debugger.initialize(hostContext) end

---Shuts down the debugger.<br/>
---The debugger is automatically shutdown during the call to Rml::Shutdown(), calling this is only necessary to shutdown the debugger early
---or to re-initialize the debugger on another host context.
function ui.Debugger.shutdown() end

---Sets the context to be debugged.
---@param context ui.Context # The context to be debugged.
---@return boolean # True if the debugger is initialised and the context was switched, false otherwise.
function ui.Debugger.setContext(context) end

---Sets the visibility of the debugger.
---@param visibility boolean # True to show the debugger, false to hide it.
function ui.Debugger.setVisible(visibility) end

---Returns the visibility of the debugger.
---@return boolean # True if the debugger is visible, false if not.
function ui.Debugger.isVisible() end
