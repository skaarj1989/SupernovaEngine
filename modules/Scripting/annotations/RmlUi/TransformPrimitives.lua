---@meta

ui.Transforms = {}

---@class ui.Transforms.NumericValue
---@field number number
---@field unit ui.PropertyUnit
---@overload fun(): ui.Transforms.NumericValue
---@overload fun(number: number, unit: ui.PropertyUnit): ui.Transforms.NumericValue
ui.Transforms.NumericValue = {}

---@class ui.Transforms.Matrix2D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Matrix2D
ui.Transforms.Matrix2D = {}

---@class ui.Transforms.Matrix3D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Matrix3D
ui.Transforms.Matrix3D = {}

---@class ui.Transforms.TranslateX
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.TranslateX
---@overload fun(x: number, unit: ui.PropertyUnit): ui.Transforms.TranslateX
ui.Transforms.TranslateX = {}

---@class ui.Transforms.TranslateY
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.TranslateY
---@overload fun(x: number, unit: ui.PropertyUnit): ui.Transforms.TranslateY
ui.Transforms.TranslateY = {}

---@class ui.Transforms.TranslateZ
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.TranslateZ
---@overload fun(x: number, unit: ui.PropertyUnit): ui.Transforms.TranslateZ
ui.Transforms.TranslateZ = {}

---@class ui.Transforms.Translate2D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Translate2D
---@overload fun(x: number, y: number, unit: ui.PropertyUnit): ui.Transforms.Translate2D
ui.Transforms.Translate2D = {}

---@class ui.Transforms.Translate3D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Translate3D
---@overload fun(x: number, y: number, z: number, unit: ui.PropertyUnit): ui.Transforms.Translate3D
ui.Transforms.Translate3D = {}

---@class ui.Transforms.ScaleX
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.ScaleX
---@overload fun(values: number): ui.Transforms.ScaleX
ui.Transforms.ScaleX = {}

---@class ui.Transforms.ScaleY
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.ScaleY
---@overload fun(values: number): ui.Transforms.ScaleY
ui.Transforms.ScaleY = {}

---@class ui.Transforms.ScaleZ
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.ScaleZ
---@overload fun(values: number): ui.Transforms.ScaleZ
ui.Transforms.ScaleZ = {}

---@class ui.Transforms.Scale2D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Scale2D
---@overload fun(xy: number): ui.Transforms.Scale2D
---@overload fun(x: number, y: number): ui.Transforms.Scale2D
ui.Transforms.Scale2D = {}

---@class ui.Transforms.Scale3D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Scale3D
---@overload fun(xyz: number): ui.Transforms.Scale3D
---@overload fun(x: number, y: number, z: number): ui.Transforms.Scale3D
ui.Transforms.Scale3D = {}

---@class ui.Transforms.RotateX
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.RotateX
---@overload fun(angle: number, unit: ui.PropertyUnit): ui.Transforms.RotateX
ui.Transforms.RotateX = {}

---@class ui.Transforms.RotateY
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.RotateY
---@overload fun(angle: number, unit: ui.PropertyUnit): ui.Transforms.RotateY
ui.Transforms.RotateY = {}

---@class ui.Transforms.RotateZ
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.RotateZ
---@overload fun(angle: number, unit: ui.PropertyUnit): ui.Transforms.RotateZ
ui.Transforms.RotateZ = {}

---@class ui.Transforms.Rotate2D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Rotate2D
---@overload fun(angle: number, unit: ui.PropertyUnit): ui.Transforms.Rotate2D
ui.Transforms.Rotate2D = {}

---@class ui.Transforms.Rotate3D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Rotate3D
---@overload fun(xy: number, y: number, z: number, angle: number, unit: ui.PropertyUnit): ui.Transforms.Rotate3D
ui.Transforms.Rotate3D = {}

---@class ui.Transforms.SkewX
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.SkewX
---@overload fun(angle: number, unit: ui.PropertyUnit): ui.Transforms.SkewX
ui.Transforms.SkewX = {}

---@class ui.Transforms.SkewY
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.SkewY
---@overload fun(angle: number, unit: ui.PropertyUnit): ui.Transforms.SkewY
ui.Transforms.SkewY = {}

---@class ui.Transforms.Skew2D
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Skew2D
---@overload fun(x: number, y: number, unit: ui.PropertyUnit): ui.Transforms.Skew2D
ui.Transforms.Skew2D = {}

---@class ui.Transforms.Perspective
---@overload fun(values: ui.Transforms.NumericValue[]): ui.Transforms.Perspective
ui.Transforms.Perspective = {}

---@class ui.Transforms.DecomposedMatrix4
ui.Transforms.DecomposedMatrix4 = {}

---@enum ui.Transforms.TransformPrimitiveType
ui.Transforms.TransformPrimitiveType = {
    MATRIX2D = 0,
    MATRIX3D = 1,
    TRANSLATEX = 2,
    TRANSLATEY = 3,
    TRANSLATEZ = 4,
    TRANSLATE2D = 5,
    TRANSLATE3D = 6,
    SCALEX = 7,
    SCALEY = 8,
    SCALEZ = 9,
    SCALE2D = 10,
    SCALE3D = 11,
    ROTATEX = 12,
    ROTATEY = 13,
    ROTATEZ = 14,
    ROTATE2D = 15,
    ROTATE3D = 16,
    SKEWX = 17,
    SKEWY = 18,
    SKEW2D = 19,
    PERSPECTIVE = 20,
    DECOMPOSEDMATRIX4 = 21,
}

---@class ui.Transforms.TransformPrimitive
---@field type ui.Transforms.TransformPrimitiveType
---@overload fun(p: ui.Transforms.Matrix2D|ui.Transforms.Matrix3D|ui.Transforms.TranslateX|ui.Transforms.TranslateY|ui.Transforms.TranslateZ|ui.Transforms.Translate2D|ui.Transforms.Translate3D|ui.Transforms.ScaleX|ui.Transforms.ScaleY|ui.Transforms.ScaleZ|ui.Transforms.Scale2D|ui.Transforms.Scale3D|ui.Transforms.SkewX|ui.Transforms.SkewY|ui.Transforms.Skew2D|ui.Transforms.Perspective|ui.Transforms.DecomposedMatrix4): ui.Transforms.TransformPrimitive
ui.Transforms.TransformPrimitive = {}

---@alias ui.PrimitiveList ui.Transforms.TransformPrimitive[]

---@class ui.Transform
---@overload fun(primitives?: ui.PrimitiveList) ui.Transform
ui.Transform = {}

---Helper function to create a 'transform' Property from the given list of primitives.
---@param primitives ui.PrimitiveList
---@return ui.Property
function ui.Transform.makeProperty(primitives) end

---Remove all Primitives from this Transform.
function ui.Transform:clearPrimitives() end

---Add a Primitive to this Transform
---@param p ui.Transforms.TransformPrimitive
function ui.Transform:addPrimitive(p) end

---Return the number of Primitives in this Transform.
---@return integer
function ui.Transform:getNumPrimitives() end

---Return the i-th Primitive in this Transform.
---@param i integer
---@return ui.Transforms.TransformPrimitive
function ui.Transform:getPrimitive(i) end

---@return ui.PrimitiveList
function ui.Transform:getPrimitives() end
