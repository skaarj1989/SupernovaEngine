---@meta

---@class ui.EventListener
ui.EventListerner = {}

---Process the incoming Event.
---@param event ui.Event
function ui.EventListerner:processEvent(event) end

---Called when the listener has been attached to a new Element.
---@param element ui.Element
function ui.EventListerner:onAttach(element) end

---Called when the listener has been detached from an Element.
---@param element ui.Element
function ui.EventListerner:onDetach(element) end
