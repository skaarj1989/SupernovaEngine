return FunctionInfo({
    category = "Common",
    name = "tileAndOffset",
    description = "",
    signature = "vec2 tileAndOffset(vec2, vec2, float)",
    args = {
        Parameter({
            name = "uv",
            description = "",
            attribute = Attribute.TexCoord0,
        }),
        Parameter({
            name = "tiling",
            description = "",
            value = math.vec2(1.0),
        }),
        Parameter({
            name = "offset",
            description = "",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local uv, tiling, offset = args[1], args[2], args[3]
        return (uv == DataType.Vec2
                and tiling == DataType.Vec2
                and offset == DataType.Float
            )
            and DataType.Vec2 or DataType.Undefined
    end,
})
