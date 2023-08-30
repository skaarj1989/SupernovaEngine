return FunctionInfo({
    category = "Noise",
    name = "cellular2x2",
    description = "",
    signatures = {
        "vec2 cellular2x2(vec2)",
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
        return P == DataType.Vec2 and P or DataType.Undefined
    end,
})
