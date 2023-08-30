return FunctionInfo({
    category = "Texture",
    name = "textureSize",
    description = "retrieve the dimensions of a level of a texture",
    signatures = {
        "ivec2 textureSize(gsampler2D, int)",
        "ivec2 textureSize(gsamplerCube, int)",
    },
    args = {
        Parameter({
            name = "sampler",
            description = "Specifies the sampler to which the texture whose dimensions to retrieve is bound.",
            sampler = TextureParam(),
        }),
        Parameter({
            name = "lod",
            description = "Specifies the level of the texture for which to retrieve the dimensions.",
            value = 0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local sampler, lod = args[1], args[2]
        return (isSampler(sampler) and lod == DataType.Int32)
            and DataType.IVec2 or DataType.Undefined
    end,
})
