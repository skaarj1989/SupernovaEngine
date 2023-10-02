---@meta

ui.Style = {}

---@enum ui.Style.Display
ui.Style.Display = {
  None = 0,
  Block = 1,
  Inline = 2,
  InlineBlock = 3,
  Flex = 4,
  Table = 5,
  TableRow = 6,
  TableRowGroup = 7,
  TableColumn = 8,
  TableColumnGroup = 9,
  TableCell = 10,
}

---@enum ui.Style.Position
ui.Style.Position = {
  Static = 0, Relative = 1, Absolute = 2, Fixed = 3,
}

---@enum ui.Style.Float
ui.Style.Float = {
  None = 0, Left = 1, Right = 2,
}

---@enum ui.Style.Clear
ui.Style.Clear = {
  None = 0, Left = 1, Right = 2, Both = 3,
}

---@enum ui.Style.BoxSizing
ui.Style.BoxSizing = {
  ContentBox = 0, BorderBox = 1,
}

---@enum ui.Style.Overflow
ui.Style.Overflow = {
  Visible = 0, Hidden = 1, Auto = 2, Scroll = 3,
}

---@enum ui.Style.Visibility
ui.Style.Visibility = {
  Visible = 0, Hidden = 1,
}

---@enum ui.Style.FontStyle
ui.Style.FontStyle = {
  Normal = 0, Italic = 1,
}

---@enum ui.Style.FontWeight
ui.Style.FontWeight = {
  Auto = 0, Normal = 400, Bold = 700,
}

---@enum ui.Style.TextAlign
ui.Style.TextAlign = {
  Left = 0, Right = 1, Center = 2, Justify = 3,
}

---@enum ui.Style.TextDecoration
ui.Style.TextDecoration = {
  None = 0, Underline = 1, Overline = 2, LineThrough = 3,
}

---@enum ui.Style.TextTransform
ui.Style.TextTransform = {
  None = 0, Capitalize = 1, Uppercase = 2, Lowercase = 3,
}

---@enum ui.Style.WhiteSpace
ui.Style.WhiteSpace = {
  Normal = 0, Pre = 1, Nowrap = 2, Prewrap = 3, Preline = 4,
}

---@enum ui.Style.WordBreak
ui.Style.WordBreak = {
  Normal = 0, BreakAll = 1, BreakWord = 2,
}

---@enum ui.Style.Drag
ui.Style.Drag = {
  None = 0, Drag = 1, DragDrop = 2, Block = 3, Clone = 4,
}

---@enum ui.Style.TabIndex
ui.Style.TabIndex = {
  None = 0, Auto = 1,
}

---@enum ui.Style.Focus
ui.Style.Focus = {
  None = 0, Auto = 1,
}

---@enum ui.Style.OverscrollBehavior
ui.Style.OverscrollBehavior = {
  Auto = 0, Contain = 1,
}

---@enum ui.Style.PointerEvents
ui.Style.PointerEvents = {
  None = 0, Auto = 1,
}

---@enum ui.Style.OriginX
ui.Style.OriginX = {
  Left = 0, Center = 1, Right = 2,
}

---@enum ui.Style.OriginY
ui.Style.OriginY = {
  Top = 0, Center = 1, Bottom = 2,
}

---@enum ui.Style.AlignContent
ui.Style.AlignContent = {
  FlexStart = 0,
  FlexEnd = 1,
  Center = 2,
  SpaceBetween = 3,
  SpaceAround = 4,
  Stretch = 5,
}

---@enum ui.Style.AlignItems
ui.Style.AlignItems = {
  FlexStart = 0,
  FlexEnd = 1,
  Center = 2,
  Baseline = 3,
  Stretch = 4,
}

---@enum ui.Style.AlignSelf
ui.Style.AlignSelf = {
  Auto = 0,
  FlexStart = 1,
  FlexEnd = 2,
  Center = 3,
  Baseline = 4,
  Stretch = 5,
}

---@enum ui.Style.FlexDirection
ui.Style.FlexDirection = {
  Row = 0, RowReverse = 1, Column = 2, ColumnReverse = 3,
}

---@enum ui.Style.FlexWrap
ui.Style.FlexWrap = {
  Nowrap = 0, Wrap = 1, WrapReverse = 2,
}

---@enum ui.Style.JustifyContent
ui.Style.JustifyContent = {
  FlexStart = 0,
  FlexEnd = 1,
  Center = 2,
  SpaceBetween = 3,
  SpaceAround = 4,
}

---@enum ui.BoxArea
ui.BoxArea = {
  MARGIN = 0,
  BORDER = 1,
  PADDING = 2,
  CONTENT = 3,
  NUM_AREAS = 3,
}

---@enum ui.BoxEdge
ui.BoxEdge = {
  TOP = 0,
  RIGHT = 1,
  BOTTOM = 2,
  LEFT = 3,
  NUM_EDGES = 4,
}

---@enum ui.BoxDirection
ui.BoxDirection = {
  VERTICAL = 0, HORIZONTAL = 1,
}

---@class ui.Box
---@overload fun(content?: vec2): ui.Box
ui.Box = {}

---Returns the top-left position of one of the box's areas, relative to the top-left of the border area. This
---means the position of the margin area is likely to be negative.
---@param area? ui.BoxArea # The desired area.
---@return vec2 # The position of the area.
function ui.Box:getPosition(area) end

---Returns the size of one of the box's areas. This will include all inner areas.
---@param area ui.BoxArea # The desired area.
---@return vec2 # The size of the requested area.
---@overload fun(): vec2 # The size of the content area.
function ui.Box:getSize(area) end

---Sets the size of the content area.
---@param content vec2 @ The size of the new content area.
function ui.Box:setContent(content) end

---Sets the size of one of the edges of one of the box's outer areas.
---@param area ui.BoxArea # The area to change.
---@param edge ui.BoxEdge # The area edge to change.
---@param size number # The new size of the area segment.
function ui.Box:setEdge(area, edge, size) end

---Returns the size of one of the area edges.
---@param area ui.BoxArea # The desired area.
---@param edge ui.BoxEdge # The desired edge.
---@return number # The size of the requested area edge.
function ui.Box:getEdge(area, edge) end

---Returns the cumulative size of one edge up to one of the box's areas.
---@param area ui.BoxArea # The area to measure up to (and including). So, MARGIN will return the width of the margin, and PADDING will be the sum of the margin, border and padding.
---@param edge ui.BoxEdge # The desired edge.
---@return number # The cumulative size of the edge.
function ui.Box:getCumulativeEdge(area, edge) end

---Returns the size along a single direction of the given 'area', including all inner areas up-to and including 'area_end'.<br/>
---EXAMPLE: getSizeAcross(HORIZONTAL, BORDER, PADDING) returns the total width of the horizontal borders and paddings.
---@param direction ui.BoxDirection # The desired direction.
---@param area ui.BoxArea # The widest area to include.
---@param areaEnd? ui.BoxArea # The last area to include, anything inside this is excluded.
function ui.Box:getSizeAcross(direction, area, areaEnd) end

---@class ui.StyleSheet
ui.StyleSheet = {}

---@class ui.StyleSheetSpecification
ui.StyleSheetSpecification = {}

---Parses a property declaration, setting any parsed and validated properties on the given dictionary.
---@param dictionary ui.PropertyDictionary # The property dictionary which will hold all declared properties.
---@param propertyName string # The name of the declared property.
---@param propertyValue string # The values the property is being set to.
---@return boolean # True if all properties were parsed successfully, false otherwise.
function ui.StyleSheetSpecification.parsePropertyDeclaration(dictionary, propertyName, propertyValue) end

---@param name string
---@return ui.PropertyId
function ui.StyleSheetSpecification.getPropertyId(name) end

---@param name string
---@return ui.ShorthandId
function ui.StyleSheetSpecification.getShorthandId(name) end

---@param id ui.PropertyId
---@return string
function ui.StyleSheetSpecification.getPropertyName(id) end

---@param id ui.ShorthandId
---@return string
function ui.StyleSheetSpecification.getShorthandName(id) end
