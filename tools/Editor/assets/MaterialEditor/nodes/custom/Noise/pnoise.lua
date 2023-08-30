return FunctionInfo({
    category = "Noise",
    name = "pnoise",
    description = "",
    signatures = {
        "float pnoise(vec2, vec2)",
        "float pnoise(vec3, vec3)",
        "float pnoise(vec4, vec4)",
    },
    args = {
        Parameter({
            name = "P",
            description = "",
            value = math.vec2(0.0),
        }),
        Parameter({
            name = "rep",
            description = "",
            value = math.vec2(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local P, rep = args[1], args[2]
        return (P == rep and vec:find(P))
            and DataType.Float or DataType.Undefined
    end,
})
