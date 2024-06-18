---@meta

---@enum ui.ShorthandId
ui.ShorthandId = {
  Invalid = 0,

  ---The following values define the shorthand ids for the main stylesheet specification.
  ---These values must not be used in places that have their own property specification,
  ---such as decorators and font-effects.

  Margin = 1,
  Padding = 2,
  BorderWidth = 3,
  BorderColor = 4,
  BorderTop = 5,
  BorderRight = 6,
  BorderBottom = 7,
  BorderLeft = 8,
  Border = 9,
  BorderRadius = 10,
  Overflow = 11,
  Background = 12,
  Font = 13,
  Gap = 14,
  PerspectiveOrigin = 15,
  TransformOrigin = 16,
  Flex = 17,
  FlexFlow = 18,

  NumDefinedIds = 19,
  FirstCustomId = ui.ShorthandId.NumDefinedIds,

  ---The maximum number of IDs. This limits the number of possible custom IDs to MaxNumIds - FirstCustomId.
  MaxNumIds = 0xff,
}

---@enum ui.PropertyId
ui.PropertyId = {
  Invalid = 0,

  ---The following values define the property ids for the main stylesheet specification.
  ---These values must not be used in places that have their own property specification,
  ---such as decorators and font-effects.

  MarginTop = 1,
  MarginRight = 2,
  MarginBottom = 3,
  MarginLeft = 4,
  PaddingTop = 5,
  PaddingRight = 6,
  PaddingBottom = 7,
  PaddingLeft = 8,
  BorderTopWidth = 9,
  BorderRightWidth = 10,
  BorderBottomWidth = 11,
  BorderLeftWidth = 12,
  BorderTopColor = 13,
  BorderRightColor = 14,
  BorderBottomColor = 15,
  BorderLeftColor = 16,
  BorderTopLeftRadius = 17,
  BorderTopRightRadius = 18,
  BorderBottomRightRadius = 19,
  BorderBottomLeftRadius = 20,
  Display = 21,
  Position = 22,
  Top = 23,
  Right = 24,
  Bottom = 25,
  Left = 26,
  Float = 27,
  Clear = 28,
  BoxSizing = 29,
  ZIndex = 30,
  Width = 31,
  MinWidth = 32,
  MaxWidth = 33,
  Height = 34,
  MinHeight = 35,
  MaxHeight = 36,
  LineHeight = 37,
  VerticalAlign = 38,
  OverflowX = 39,
  OverflowY = 40,
  Clip = 41,
  Visibility = 42,
  BackgroundColor = 43,
  Color = 44,
  CaretColor = 45,
  ImageColor = 46,
  FontFamily = 47,
  FontStyle = 48,
  FontWeight = 49,
  FontSize = 50,
  TextAlign = 51,
  TextDecoration = 52,
  TextTransform = 53,
  WhiteSpace = 54,
  WordBreak = 55,
  RowGap = 56,
  ColumnGap = 57,
  Cursor = 58,
  Drag = 59,
  TabIndex = 60,
  ScrollbarMargin = 61,
  OverscrollBehavior = 62,

  Perspective = 63,
  PerspectiveOriginX = 64,
  PerspectiveOriginY = 65,
  Transform = 66,
  TransformOriginX = 67,
  TransformOriginY = 68,
  TransformOriginZ = 69,

  Transition = 70,
  Animation = 71,

  Opacity = 72,
  PointerEvents = 73,
  Focus = 74,

  Decorator = 75,
  FontEffect = 76,

  FillImage = 77,

  AlignContent = 78,
  AlignItems = 79,
  AlignSelf = 80,
  FlexBasis = 81,
  FlexDirection = 82,
  FlexGrow = 83,
  FlexShrink = 84,
  FlexWrap = 85,
  JustifyContent = 86,

  NumDefinedIds = 87,
  FirstCustomId = ui.PropertyId.NumDefinedIds,

  ---The maximum number of IDs. This limits the number of possible custom IDs to MaxNumIds - FirstCustomId.
  MaxNumIds = 128,
}

---@enum ui.EventId
ui.EventId = {
  Invalid = 0,

  --- Core events:

  Mousedown = 1,
  Mousescroll = 2,
  Mouseover = 3,
  Mouseout = 4,
  Focus = 5,
  Blur = 6,
  Keydown = 7,
  Keyup = 8,
  Textinput = 9,
  Mouseup = 10,
  Click = 11,
  Dblclick = 12,
  Load = 13,
  Unload = 14,
  Show = 15,
  Hide = 16,
  Mousemove = 17,
  Dragmove = 18,
  Drag = 19,
  Dragstart = 20,
  Dragover = 21,
  Dragdrop = 22,
  Dragout = 23,
  Dragend = 24,
  Handledrag = 25,
  Resize = 26,
  Scroll = 27,
  Animationend = 28,
  Transitionend = 29,

  --- Form control events:

  Change = 30,
  Submit = 31,
  Tabchange = 32,

  NumDefinedIds = 33,

  --- Custom IDs start here.
  FirstCustomId = ui.EventId.NumDefinedIds,

  --- The maximum number of IDs. This limits the number of possible custom IDs to MaxNumIds - FirstCustomId.
  MaxNumIds = 0xffff,
}
