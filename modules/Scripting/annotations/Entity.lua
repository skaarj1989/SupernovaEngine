---@meta

---@class Entity
Entity = {}

---@return integer
function Entity:id() end

---@return boolean
function Entity:valid() end

---@generic T
---@param type T
---@return boolean
---@overload fun(id: integer): boolean
function Entity:has(type) end

---@generic T
---@param type T
---@return T
---@overload fun(id: integer): userdata
function Entity:get(type) end

---@generic T
---@param component T
---@return T
function Entity:emplace(component) end

---@generic T
---@param type T
---@return boolean
---@overload fun(id: integer): boolean
function Entity:remove(type) end

function Entity:destroy() end

---@return Entity
function Entity:getParent() end

---@return Entity[]
function Entity:getChildren() end

---@param entity Entity
function Entity:attach(entity) end

---@param newParent Entity
function Entity:attachTo(newParent) end

function Entity:detach() end
