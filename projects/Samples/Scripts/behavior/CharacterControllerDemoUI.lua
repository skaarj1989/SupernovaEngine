require "UI.Fonts.LatoLatin"

---@class CharacterControllerDemoUI : ScriptNode
local node = {}

function node:init()
  self.uic = self.entity:get(UIComponent).context

  self.data = {
    glitch = 0.0,
  }
  local dataModel = self.uic:bindDataModel("data", self.data)
  self.dataModelHandle = dataModel:getModelHandle()

  self.doc = self.uic:loadDocument("UI/CharacterControllerDemoUI.rml")
  self.doc:show()
end

---@param evt InputEvent
function node:input(evt)
  if inEditor and isTypeOf(evt, MouseMoveEvent) then
    local extent <const> = getPreviewExtent()
    evt.position = ui.scaleMousePosition(
      evt.position,
      math.ivec2(extent.width, extent.height),
      self.uic:getDimensions()
    )
  end
  ui.processEvent(self.uic, evt)
end

---@param dt number
function node:update(dt)
  if self.dataModelHandle:isVariableDirty("glitch") then
    local effect <const> = self.entity:get(CameraComponent).postProcessEffects[1]
    effect:setProperty("fs_strength", self.data.glitch)
  end
end

return node
