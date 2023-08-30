return FunctionInfo({
    category = "Texture",
    name = "getTexelSize",
    description = "",
    signatures = {
        "vec2 getTexelSize(sampler2D)",
        "vec3 getTexelSize(samplerCube)",
    },
    args = {
        Parameter({
            name = "sampler",
            description = "",
            sampler = TextureParam(),
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local sampler = args[1]
        if sampler == DataType.Sampler2D then
            return DataType.Vec2
        elseif sampler == DataType.SamplerCube then
            return DataType.Vec3
        end
        return DataType.Undefined
    end,
})
