---@meta

---@enum ui.EventPhase
ui.EventPhase = {
  None = 0, Capture = 1, Target = 2, Bubble = 4,
}

---@class ui.Event
---@overload fun(): ui.Event
---@overload fun(target: ui.Element, id: ui.EventId, type: string, parameters: ui.Dictionary, interruptible: boolean): ui.Event
ui.Event = {}

---Get the current propagation phase.
---@return ui.EventPhase
function ui.Event:getPhase() end

---Set the current propagation phase.
---@param phase ui.EventPhase
function ui.Event:setPhase(phase) end

---Set the current element in the propagation.
---@param element ui.Element
function ui.Event:setCurrentElement(element) end

---Get the current element in the propagation.
---@return ui.Element
function ui.Event:getCurrentElement() end

---Get the target element of this event.
---@return ui.Element
function ui.Event:getTargetElement() end

---Get the event type.
---@return string
function ui.Event:getType() end

---Get the event id.
---@return ui.EventId
function ui.Event:getId() end

---Stops propagation of the event if it is interruptible, but finish all listeners on the current element.
function ui.Event:stopPropagation() end

---Stops propagation of the event if it is interruptible, including to any other listeners on the current element.
function ui.Event:stopImmediatePropagation() end

---Returns true if the event can be interrupted, that is, stopped from propagating.
---@return boolean
function ui.Event:isInterruptible() end

---Returns true if the event is still propagating.
---@return boolean
function ui.Event:isPropagating() end

---Returns true if the event is still immediate propagating.
---@return boolean
function ui.Event:isImmediatePropagating() end

---Returns the value of one of the event's parameters.
---@param key string # The name of the desired parameter.
---@return ui.Variant # The value of the requested parameter.
function ui.Event:getParameter(key) end

---Return the unprojected mouse screen position.<br/>
---NOTE: Only specified for events with 'mouse_x' and 'mouse_y' parameters.
---@return vec2
function ui.Event:getUnprojectedMouseScreenPos() end
