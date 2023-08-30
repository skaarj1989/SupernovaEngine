return FunctionInfo({
    category = "Noise",
    name = "snoise",
    description = "",
    signatures = {
        "float snoise(vec2)",
        "float snoise(vec3)",
        "float snoise(vec4)",
    },
    args = {
        Parameter({
            name = "P",
            description = "",
            value = math.vec2(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local P = args[1]
        return vec:find(P) and DataType.Float or DataType.Undefined
    end,
})
