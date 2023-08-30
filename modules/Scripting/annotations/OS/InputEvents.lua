--- @meta

--- @class KeyboardEvent : EventBase
--- @field state KeyState
--- @field keyCode KeyCode
--- @field charCode integer
--- @field repeated boolean
KeyboardEvent = {}

--- @enum KeyState
KeyState = {
    Up = 0,
    Down = 1,
}

--- @enum KeyCode
KeyCode = {
    Backspace = 0x08,
    Tab = 0x09,
    Clear = 0x0C,
    Return = 0x0D,
    Shift = 0x10,
    Control = 0x11,
    Menu = 0x12,
    Pause = 0x13,
    Captial = 0x14,
    Esc = 0x1B,
    Space = 0x20,

    Prior = 0x21,
    Next = 0x22,
    End = 0x23,
    Home = 0x24,

    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,

    Snapshot = 0x2C,
    Insert = 0x2D,
    Delete = 0x2E,

    LWin = 0x5,
    RWin = 0x5,
    Apps = 0x5D,

    _0 = 0x30,
    _1 = 0x31,
    _2 = 0x32,
    _3 = 0x33,
    _4 = 0x34,
    _5 = 0x35,
    _6 = 0x36,
    _7 = 0x37,
    _8 = 0x38,
    _9 = 0x39,

    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

    Num0 = 0x60,
    Num1 = 0x61,
    Num2 = 0x62,
    Num3 = 0x63,
    Num4 = 0x64,
    Num5 = 0x65,
    Num6 = 0x66,
    Num7 = 0x67,
    Num8 = 0x68,
    Num9 = 0x69,
    Multiply = 0x6A,
    Add = 0x6B,
    Separator = 0x6C,
    Subtract = 0x6D,
    Decimal = 0x6F,
    Divide = 0x70,

    F1 = 0x70,
    F2 = 0x71,
    F3 = 0x72,
    F4 = 0x73,
    F5 = 0x74,
    F6 = 0x75,
    F7 = 0x76,
    F8 = 0x77,
    F9 = 0x78,
    F10 = 0x79,
    F11 = 0x7A,
    F12 = 0x7B,

    NumLock = 0x90,
    SLock = 0x91,

    LShift = 0xA0,
    RShift = 0xA1,
    LControl = 0xA2,
    RControl = 0xA3,
    LMenu = 0xA4,
    RMenu = 0xA5,
}

--- @class InputCharacterEvent : EventBase
--- @field c integer
InputCharacterEvent = {}

--- @class MouseMoveEvent : EventBase
--- @field position ivec2
MouseMoveEvent = {}

--- @class MouseButtonEvent : MouseMoveEvent
--- @field state MouseButtonState
--- @field button MouseButton
MouseButtonEvent = {}

--- @enum MouseButton
MouseButton = {
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,
    X2 = 4,
}

--- @enum MouseButtonState
MouseButtonState = {
    Pressed = 0,
    Released = 1,
    DblClick = 2,
}

--- @class MouseWheelEvent : EventBase
--- @field wheel MouseWheel
--- @field step number
MouseWheelEvent = {}

--- @enum MouseWheel
MouseWheel = {
    Vertical = 0,
    Horizontal = 1,
}

--- @alias InputEvent MouseMoveEvent|MouseButtonEvent|MouseWheelEvent|KeyboardEvent|InputCharacterEvent
