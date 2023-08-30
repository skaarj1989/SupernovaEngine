return FunctionInfo({
    category = "Mathematics",
    name = "log",
    description = "return the natural logarithm of the parameter",
    signature = "genType log(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value of which to take the natural logarithm.",
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
