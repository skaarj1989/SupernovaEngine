return FunctionInfo({
    category = "Noise",
    name = "cnoise",
    description = "",
    signatures = {
        "float cnoise(vec2)",
        "float cnoise(vec3)",
        "float cnoise(vec4)",
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
