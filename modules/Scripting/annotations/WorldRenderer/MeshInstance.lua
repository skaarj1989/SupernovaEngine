--- @meta

--- @class MeshInstance : ComponentBase
--- @overload fun(resource: MeshResource): MeshInstance
MeshInstance = {}

--- @param b boolean
--- @return self
function MeshInstance:show(b) end

--- @return integer Number of visible sub-meshes
function MeshInstance:countVisible() end

--- @return MeshResource
function MeshInstance:getResource() end

--- @return MeshResource
--- @param index integer
--- @param material MaterialResource
function MeshInstance:setMaterial(index, material) end

--- @return MaterialInstance
--- @param index integer
function MeshInstance:getMaterial(index) end

--- @return boolean
function MeshInstance:hasSkin() end

--- @return MeshInstance self
function MeshInstance:reset() end

--- @class DecalInstance : MeshInstance
--- @overload fun(resource: MeshResource): DecalInstance
DecalInstance = {}
