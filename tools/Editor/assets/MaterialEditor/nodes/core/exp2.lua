return FunctionInfo({
    category = "Mathematics",
    name = "exp2",
    description = "return 2 raised to the power of the parameter",
    signature = "genType exp2(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value of the power to which 2 will be raised.",
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
