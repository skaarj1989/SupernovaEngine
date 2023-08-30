return FunctionInfo({
    category = nil,
    name = "blur13",
    description = "",
    signature = "vec4 blur5(sampler2D, vec2, vec2, vec2)",
    args = {
        Parameter({
            name = "sampler",
            description = "",
            sampler = TextureParam(),
        }),
        Parameter({
            name = "P",
            description = "",
            attribute = Attribute.TexCoord0,
        }),
        Parameter({
            name = "resolution",
            description = "",
            value = math.vec2(),
        }),
        Parameter({
            name = "direction",
            description = "",
            value = math.vec2(),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 4)
        local image, uv, resolution, direction = args[1], args[2], args[3], args[4]
        return (image == DataType.Sampler2D
                and uv == DataType.Vec2
                and (resolution == DataType.Vec2 or resolution == DataType.IVec2 or resolution == DataType.UVec2)
                and (direction == DataType.Vec2 or direction == DataType.IVec2 or direction == DataType.UVec2)
            )
            and DataType.Vec4 or DataType.Undefined
    end,
})
