require "UI.Fonts.LatoLatin"

---@class SkeletalAnimationDemoHUD : ScriptNode
local node = {}

function node:init()
  self.uic = self.entity:get(UIComponent).context

  local data <const> = {
    ---@param evt ui.Event
    switchAnimation = function(dataModelHandle, evt, args)
      local path <const> = "Meshes/Mutant/animations/" .. evt:getParameter("value"):get() .. ".ozz"
      local mutant <const> = getEntity(5)
      mutant:get(AnimationComponent).resource = loadAnimation(path)
      mutant:get(PlaybackController):stop():play()
    end
  }
  local dataModel = self.uic:bindDataModel("data", data)
  self.dataModelHandle = dataModel:getModelHandle()

  self.doc = self.uic:loadDocument("UI/SkeletalAnimationDemoHUD.rml")
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

return node
