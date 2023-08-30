return FunctionInfo({
    category = "Trigonometry",
    name = "atan",
    description = "return the arc-tangent of the parameters",
    signature = "genType atan(genType, genType)",
    args = {
        Parameter({
            name = "y",
            description = "Specify the numerator of the fraction whose arctangent to return.",
            value = 1.0,
        }),
        Parameter({
            name = "x",
            description = "Specify the denominator of the fraction whose arctangent to return.",
            defaultValue = 1.0,
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local y, x = args[1], args[2]
        return (y == x and genType:find(y))
            and y or DataType.Undefined
    end,
})
