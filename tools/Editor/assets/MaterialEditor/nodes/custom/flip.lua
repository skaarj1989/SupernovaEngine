return FunctionInfo({
    category = nil,
    name = "flip",
    description = "",
    signatures = {
        "vec2 flip(vec2, bvec2)",
        "vec3 flip(vec3, bvec3)",
        "vec4 flip(vec4, bvec4)",
    },
    args = {
        Parameter({
            name = "in",
            description = "",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "flip",
            description = "",
            value = math.bvec3(false),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local in_, flip = args[1], args[2]
        return (countChannels(in_) == countChannels(flip) and
            vec:find(in_) and bvec:find(flip)) and in_ or DataType.Undefined
    end,
})
