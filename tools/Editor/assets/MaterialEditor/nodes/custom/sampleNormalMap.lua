return FunctionInfo({
    category = "Texture",
    name = "sampleNormalMap",
    description = "",
    signature = "vec3 sampleNormalMap(Sampler2D, vec2)",
    args = {
        Parameter({
            name = "normalMap",
            description = "",
            sampler = TextureParam(),
        }),
        Parameter({
            name = "uv",
            description = "TexCoord",
            attribute = Attribute.TexCoord0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local normalMap, uv = args[1], args[2]
        return (normalMap == DataType.Sampler2D and uv == DataType.Vec2)
            and DataType.Vec3 or DataType.Undefined
    end,
})
