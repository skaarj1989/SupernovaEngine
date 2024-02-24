---@meta

---@class Tracy
tracy = {}

function tracy.ZoneBegin() end

---@param name string
function tracy.ZoneBeginN(name) end

function tracy.ZoneEnd() end

---@param text string
function tracy.ZoneText(text) end

---@param text string
function tracy.ZoneName(text) end

---@param text string
function tracy.Message(text) end
