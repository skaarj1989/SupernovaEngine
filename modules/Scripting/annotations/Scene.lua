---@meta

-- Scene/Scene.cpp

---@return Entity
function createEntity() end

---@return Entity
---@param id integer
function getEntity(id) end

---@return PhysicsWorld
function getPhysicsWorld() end

---@return AudioWorld
function getAudioWorld() end

---@param e integer # Entity ID
---@overload fun(e: Entity)
function setMainCamera(e) end

---@param e integer # Entity ID
---@overload fun(e: Entity)
function setMainListener(e) end
