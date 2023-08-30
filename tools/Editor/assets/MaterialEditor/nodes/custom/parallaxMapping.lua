return FunctionInfo({
    category = "Texture",
    name = "parallaxMapping",
    description = "",
    signature = "vec2 parallaxOcclusionMapping(sampler2D, vec2, vec3, vec3, float)",
    args = {
        Parameter({
            name = "heightMap",
            description = "",
            sampler = TextureParam(),
        }),
        Parameter({
            name = "texCoord",
            description = "TexCoord",
            attribute = Attribute.TexCoord0,
        }),
        Parameter({
            name = "V",
            description = "Fragment to camera (world-space)",
            constant = BuiltInConstant.ViewDir,
        }),
        Parameter({
            name = "N",
            description = "Normal (world-space)",
            attribute = Attribute.Normal,
        }),
        Parameter({
            name = "scale",
            description = "",
            value = 0.1,
        }),
    },
    getReturnType = function(args)
        assert(#args == 5)
        local heightMap, texCoord, V, N, scale = args[1], args[2], args[3], args[4], args[5]
        return (heightMap == DataType.Sampler2D
                and texCoord == DataType.Vec2
                and V == DataType.Vec3
                and N == DataType.Vec3
                and scale == DataType.Float
            )
            and DataType.Vec2 or DataType.Undefined
    end,
})
