---@meta

---@enum ui.VariantType
ui.VariantType = {
  NONE = '-',
  BOOL = 'B',
  BYTE = 'b',
  CHAR = 'c',
  FLOAT = 'f',
  DOUBLE = 'd',
  INT = 'i',
  INT64 = 'I',
  UINT = 'u',
  UINT64 = 'U',
  STRING = 's',
  VECTOR2 = '2',
  VECTOR3 = '3',
  VECTOR4 = '4',
  COLOURF = 'g',
  COLOURB = 'h',
  SCRIPTINTERFACE = 'p',
  TRANSFORMPTR = 't',
  TRANSITIONLIST = 'T',
  ANIMATIONLIST = 'A',
  DECORATORSPTR = 'D',
  FONTEFFECTSPTR = 'F',
  VOIDPTR = '*',
}

---@class ui.Variant
---@overload fun(value: boolean|integer|number|string|vec2|vec3|vec4): ui.Variant
ui.Variant = {}

function ui.Variant:clear() end

---@return ui.VariantType
function ui.Variant:getType() end

---@class ui.PropertyDictionary
---@overload fun(): ui.PropertyDictionary
ui.PropertyDictionary = {}

---Sets a property on the dictionary. Any existing property with the same id will be overwritten.
---@param id ui.PropertyId
---@param property ui.Property
function ui.PropertyDictionary:setProperty(id, property) end

---Removes a property from the dictionary, if it exists.
---@param id ui.PropertyId
function ui.PropertyDictionary:removeProperty(id) end

---Returns the value of the property with the requested id, if one exists.
---@param id ui.PropertyId
function ui.PropertyDictionary:getProperty(id) end

---Returns the number of properties in the dictionary.
---@return integer
function ui.PropertyDictionary:getNumProperties() end

---Imports into the dictionary, and optionally defines the specificity of, potentially
---un-specified properties. In the case of id conflicts, the incoming properties will
---overwrite the existing properties if their specificity (or their forced specificity)
---are at least equal.
---@param propertyDictionary ui.PropertyDictionary # The properties to import.
---@param specificity? integer # The specificity for all incoming properties. If this is not specified, the properties will keep their original specificity.
function ui.PropertyDictionary:import(propertyDictionary, specificity) end

---Merges the contents of another fully-specified property dictionary with this one.
---Properties defined in the new dictionary will overwrite those with the same id as
---appropriate.
---@param propertyDictionary ui.PropertyDictionary # The dictionary to merge.
---@param specificityOffset? integer # The specificities of all incoming properties will be offset by this value.
function ui.PropertyDictionary:merge(propertyDictionary, specificityOffset) end

---@enum ui.PropertyUnit
ui.PropertyUnit = {
  UNKNOWN = 1 << 0,

  KEYWORD = 1 << 1, ---generic keyword; fetch as < int >

  STRING = 1 << 2,  ---generic string; fetch as < String >

  --- Absolute values:

  NUMBER = 1 << 3, ---number unsuffixed; fetch as < float >
  PX = 1 << 4,     ---number suffixed by 'px'; fetch as < float >
  DEG = 1 << 5,    ---number suffixed by 'deg'; fetch as < float >
  RAD = 1 << 6,    ---number suffixed by 'rad'; fetch as < float >
  COLOUR = 1 << 7, ---colour; fetch as < Colourb >
  DP = 1 << 8,     ---density-independent pixel; number suffixed by 'dp'; fetch as < float >
  X = 1 << 9,      ---dots per px unit; number suffixed by 'x'; fetch as < float >
  VW = 1 << 10,    ---viewport-width percentage; number suffixed by 'vw'; fetch as < float >
  VH = 1 << 11,    ---viewport-height percentage; number suffixed by 'vh'; fetch as < float >
  ABSOLUTE_UNIT = ui.PropertyUnit.NUMBER | ui.PropertyUnit.PX | ui.PropertyUnit.DP | ui.PropertyUnit.X |
      ui.PropertyUnit.DEG | ui.PropertyUnit.RAD | ui.PropertyUnit.COLOUR | ui.PropertyUnit.VW | ui.PropertyUnit.VH,

  --- Relative values:

  EM = 1 << 12,      ---number suffixed by 'em'; fetch as < float >
  PERCENT = 1 << 13, ---number suffixed by '%'; fetch as < float >
  REM = 1 << 14,     ---number suffixed by 'rem'; fetch as < float >
  RELATIVE_UNIT = ui.PropertyUnit.EM | ui.PropertyUnit.REM | ui.PropertyUnit.PERCENT,

  --- Values based on pixels-per-inch:

  INCH = 1 << 15, ---number suffixed by 'in'; fetch as < float >
  CM = 1 << 16,   ---number suffixed by 'cm'; fetch as < float >
  MM = 1 << 17,   ---number suffixed by 'mm'; fetch as < float >
  PT = 1 << 18,   ---number suffixed by 'pt'; fetch as < float >
  PC = 1 << 19,   ---number suffixed by 'pc'; fetch as < float >
  PPI_UNIT = ui.PropertyUnit.INCH | ui.PropertyUnit.CM | ui.PropertyUnit.MM | ui.PropertyUnit.PT | ui.PropertyUnit.PC,

  TRANSFORM = 1 << 20,  ---transform; fetch as < TransformPtr >, may be empty
  TRANSITION = 1 << 21, ---transition; fetch as < TransitionList >
  ANIMATION = 1 << 22,  ---animation; fetch as < AnimationList >
  DECORATOR = 1 << 23,  ---decorator; fetch as < DecoratorsPtr >
  FONTEFFECT = 1 << 24, ---font-effect; fetch as < FontEffectsPtr >
  RATIO = 1 << 25,      ---ratio defined as x/y; fetch as < Vector2f >

  LENGTH = ui.PropertyUnit.PX | ui.PropertyUnit.DP | ui.PropertyUnit.PPI_UNIT | ui.PropertyUnit.EM | ui.PropertyUnit.REM |
      ui.PropertyUnit.VW | ui.PropertyUnit.VH | ui.PropertyUnit.X,
  LENGTH_PERCENT = ui.PropertyUnit.LENGTH | ui.PropertyUnit.PERCENT,
  NUMBER_LENGTH_PERCENT = ui.PropertyUnit.NUMBER | ui.PropertyUnit.LENGTH | ui.PropertyUnit.PERCENT,
  ABSOLUTE_LENGTH = ui.PropertyUnit.PX | ui.PropertyUnit.DP | ui.PropertyUnit.PPI_UNIT | ui.PropertyUnit.VH |
      ui.PropertyUnit.VW | ui.PropertyUnit.X,
  ANGLE = ui.PropertyUnit.DEG | ui.PropertyUnit.RAD
}

---@class ui.Property
---@field value ui.Variant
---@field unit ui.PropertyUnit
---@overload fun(values: boolean|integer|number|string|vec2|vec2|vec4, unit: ui.PropertyUnit, specificity?: integer): ui.Property
ui.Property = {}
