return FunctionInfo({
    category = "Mathematics",
    name = "inverselerp",
    description = "",
    signature = "float inverselerp(float, float, float)",
    args = {
        Parameter({
            name = "x",
            description = "",
            value = 0.0,
        }),
        Parameter({
            name = "y",
            description = "",
            value = 1.0,
        }),
        Parameter({
            name = "v",
            description = "",
            value = 0.5,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local x, y, v = args[1], args[2], args[3]
        return (v == DataType.Float
                and x == v
                and y == v
            )
            and DataType.Float or DataType.Undefined
    end,
})
