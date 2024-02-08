---@meta

---@class CollisionStartedEvent : EventBase
---@field other integer # Entity ID
---@field offset vec3
---@field normal vec3
CollisionStartedEvent = {}

---@class CollisionEndedEvent : EventBase
---@field other integer # Entity ID
CollisionEndedEvent = {}
