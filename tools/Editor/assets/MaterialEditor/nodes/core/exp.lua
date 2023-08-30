return FunctionInfo({
    category = "Mathematics",
    name = "exp",
    description = "return the natural exponentiation of the parameter",
    signature = "genType exp(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value to exponentiate.",
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
