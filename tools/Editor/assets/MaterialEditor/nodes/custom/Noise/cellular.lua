return FunctionInfo({
    category = "Noise",
    name = "cellular",
    description = "",
    signatures = {
        "vec2 cellular(vec2)",
        "vec2 cellular(vec3)",
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
        return (P == DataType.Vec2 or P == DataType.Vec3) and DataType.Vec2 or DataType.Undefined
    end,
})
