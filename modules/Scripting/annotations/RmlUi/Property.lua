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

---@return any # boolean|integer|number|string|vec2|vec3|vec4|nil
function ui.Variant:get() end

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

---@enum ui.Unit
ui.Unit = {
  UNKNOWN = 0,

  --- Basic types.

  KEYWORD = 1 << 0, ---generic keyword; fetch as <int>
  STRING = 1 << 1,  ---generic string; fetch as <String>
  COLOUR = 1 << 2,  ---colour; fetch as <Colourb>
  RATIO = 1 << 3,   ---ratio defined as x/y; fetch as <Vector2f>

  --- Numeric values.

  NUMBER = 1 << 4,  ---number unsuffixed; fetch as <float>
  PERCENT = 1 << 5, ---number suffixed by '%'; fetch as <float>
  PX = 1 << 6,      ---number suffixed by 'px'; fetch as <float>

  --- Context-relative values.

  DP = 1 << 7, ---density-independent pixel; number suffixed by 'dp'; fetch as <float>
  VW = 1 << 8, ---viewport-width percentage; number suffixed by 'vw'; fetch as <float>
  VH = 1 << 9, ---viewport-height percentage; number suffixed by 'vh'; fetch as <float>
  X = 1 << 10, ---dots per px unit; number suffixed by 'x'; fetch as <float>

  --- Font-relative values.

  EM = 1 << 11,  ---number suffixed by 'em'; fetch as <float>
  REM = 1 << 12, ---number suffixed by 'rem'; fetch as <float>

  --- Values based on pixels-per-inch.

  INCH = 1 << 13, ---number suffixed by 'in'; fetch as <float>
  CM = 1 << 14,   ---number suffixed by 'cm'; fetch as <float>
  MM = 1 << 15,   ---number suffixed by 'mm'; fetch as <float>
  PT = 1 << 16,   ---number suffixed by 'pt'; fetch as <float>
  PC = 1 << 17,   ---number suffixed by 'pc'; fetch as <float>
  PPI_UNIT = ui.Unit.INCH | ui.Unit.CM | ui.Unit.MM | ui.Unit.PT | ui.Unit.PC,

  --- Angles.

  DEG = 1 << 18, ---number suffixed by 'deg'; fetch as <float>
  RAD = 1 << 19, ---number suffixed by 'rad'; fetch as <float>

  --- Values tied to specific types.

  TRANSFORM = 1 << 20,     ---transform; fetch as <TransformPtr>, may be empty
  TRANSITION = 1 << 21,    ---transition; fetch as <TransitionList>
  ANIMATION = 1 << 22,     ---animation; fetch as <AnimationList>
  DECORATOR = 1 << 23,     ---decorator; fetch as <DecoratorsPtr>
  FONTEFFECT = 1 << 24,    ---font-effect; fetch as <FontEffectsPtr>
  COLORSTOPLIST = 1 << 25, ---color stop list; fetch as <ColorStopList>
  BOXSHADOWLIST = 1 << 26, ---shadow list; fetch as <ShadowList>

  LENGTH = ui.Unit.PX | ui.Unit.DP | ui.Unit.VW | ui.Unit.VH | ui.Unit.EM | ui.Unit.REM | ui.Unit.PPI_UNIT,
  LENGTH_PERCENT = ui.Unit.LENGTH | ui.Unit.PERCENT,
  NUMBER_LENGTH_PERCENT = ui.Unit.NUMBER | ui.Unit.LENGTH | ui.Unit.PERCENT,
  DP_SCALABLE_LENGTH = ui.Unit.DP | ui.Unit.PPI_UNIT,
  ANGLE = ui.Unit.DEG | ui.Unit.RAD,
  NUMERIC = ui.Unit.NUMBER_LENGTH_PERCENT | ui.Unit.ANGLE | ui.Unit.X
}

---@class ui.Property
---@field value ui.Variant
---@field unit ui.Unit
---@overload fun(values: boolean|integer|number|string|vec2|vec2|vec4, unit: ui.Unit, specificity?: integer): ui.Property
ui.Property = {}
