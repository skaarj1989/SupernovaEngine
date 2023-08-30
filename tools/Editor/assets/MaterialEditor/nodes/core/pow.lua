return FunctionInfo({
    category = "Mathematics",
    name = "pow",
    description = "return the value of the first parameter raised to the power of the second",
    signature = "genType pow(genType, genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value to raise to the power y.",
            value = 2.0,
        }),
        Parameter({
            name = "y",
            description = "Specify the power to which to raise x.",
            value = 2.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local x, y = args[1], args[2]
        return (x == y and genType:find(x))
            and x or DataType.Undefined
    end,
})
