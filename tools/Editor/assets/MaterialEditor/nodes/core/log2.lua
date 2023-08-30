return FunctionInfo({
    category = "Mathematics",
    name = "log2",
    description = "return the base 2 logarithm of the parameter",
    signature = "genType log2(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value of which to take the base 2 logarithm.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return genType:find(x) and x or DataType.Undefined
    end,
})
