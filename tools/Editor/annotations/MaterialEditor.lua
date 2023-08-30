--- @meta

--- @enum DataType
DataType = {
    Undefined = 0,

    Bool = 1,
    BVec2 = 2,
    BVec3 = 3,
    BVec4 = 4,

    Int32 = 5,
    IVec2 = 6,
    IVec3 = 7,
    IVec4 = 8,

    UInt32 = 9,
    UVec2 = 10,
    UVec3 = 11,
    UVec4 = 12,

    Float = 13,
    Vec2 = 14,
    Vec3 = 15,
    Vec4 = 16,

    Double = 17,
    DVec2 = 18,
    DVec3 = 19,
    DVec4 = 20,

    Mat2 = 21,
    Mat3 = 22,
    Mat4 = 23,

    Sampler2D = 24,
    SamplerCube = 25,
}

---@type (DataType.BVec2 | DataType.BVec3 | DataType.BVec4)[]
bvec = {}
---@type (DataType.Bool | DataType.BVec2 | DataType.BVec3 | DataType.BVec4)[]
genBType = {}
--- @class genBTypeBundle
--- @field base DataType
--- @field group DataType[]
genBTypeBundle = {}

---@type (DataType.UVec2 | DataType.UVec3 | DataType.UVec4)[]
uvec = {}
---@type (DataType.UInt32 | DataType.UVec2 | DataType.UVec3 | DataType.UVec4)[]
genUType = {}
--- @class genUTypeBundle
--- @field base DataType
--- @field group DataType[]
genUTypeBundle = {}

---@type (DataType.IVec2 | DataType.IVec3 | DataType.IVec4)[]
ivec = {}
---@type (DataType.Int32 | DataType.IVec2 | DataType.IVec3 | DataType.IVec4)[]
genIType = {}
--- @class genITypeBundle
--- @field base DataType
--- @field group DataType[]
genITypeBundle = {}

---@type (DataType.Vec2 | DataType.Vec3 | DataType.Vec4)[]
vec = {}
---@type (DataType.Float | DataType.Vec2 | DataType.Vec3 | DataType.Vec4)[]
genType = {}
--- @class genTypeBundle
--- @field base DataType
--- @field group DataType[]
genTypeBundle = {}

---@type (DataType.DVec2 | DataType.DVec3 | DataType.DVec4)[]
dvec = {}

---@type (DataType.Double | DataType.DVec2 | DataType.DVec3 | DataType.DVec4)[]
genDType = {}
--- @class genDTypeBundle
--- @field base DataType
--- @field group DataType[]
genDTypeBundle = {}

--- @return boolean
--- @param type DataType
function isScalar(type) end

--- @return boolean
--- @param type DataType
function isVector(type) end

--- @return boolean
--- @param type DataType
function isMatrix(type) end

--- @return boolean
--- @param type DataType
function isSampler(type) end

--- @return DataType
--- @param type DataType
function getBaseDataType(type) end

--- @return integer
--- @param type DataType
function countChannels(type) end

--- @return integer
--- @param type DataType
function countColunns(type) end

--- @return DataType
--- @param baseType DataType
--- @param numChannels integer
function constructVectorType(baseType, numChannels) end

--- @enum Attribute
Attribute = {
    Position = 0, TexCoord0 = 1, TexCoord1 = 2, Normal = 3, Color = 4,
}

--- @param shaderType ShaderType
--- @param materialDomain MaterialDomain
function vertexShaderOnly(shaderType, materialDomain) end

--- @param shaderType ShaderType
--- @param materialDomain MaterialDomain
function fragmentShaderOnly(shaderType, materialDomain) end

--- @param shaderType ShaderType
--- @param materialDomain MaterialDomain
function surfaceOnly(shaderType, materialDomain) end

--- @param shaderType ShaderType
--- @param materialDomain MaterialDomain
function postProcessOnly(shaderType, materialDomain) end

--- @enum BuiltInConstant
BuiltInConstant = {
    CameraPosition = 0,
    ScreenTexelSize = 1,
    AspectRatio = 2,

    ModelMatrix = 3,
    FragPosWorldSpace = 4,
    FragPosViewSpace = 5,

    ViewDir = 6,
}

--- @enum BuiltInSampler
BuiltInSampler = {
    SceneDepth = 0, SceneColor = 1,
}

--- @alias ValueVariant boolean|bvec2|bvec3|bvec4|integer|ivec2|ivec3|ivec4|uvec2|uvec3|uvec4|number|vec2|vec3|vec4|mat4

--- @class TextureParam
TextureParam = {}

--- @class Parameter
--- @field value? ValueVariant
--- @field attribute? Attribute
--- @field constant? BuiltInConstant
--- @field sampler? BuiltInSampler|TextureParam
Parameter = {}

--- @class FunctionInfo
--- @field guid? integer
--- @field category? string
--- @field name string
--- @field description? string
--- @field signature? string
--- @field signatures? string[]
--- @field args? {}
FunctionInfo = {}

--- @return boolean
--- @param shaderType ShaderType
--- @param blueprint MaterialBlueprint
function FunctionInfo:isEnabled(shaderType, blueprint) end

--- @return DataType
--- @param args DataType[]
function FunctionInfo:getReturnType(args) end
