return FunctionInfo({
    category = "Mathematics",
    name = "sqrt",
    description = "return the square root of the parameter",
    signature = {
        "genType sqrt(genType)",
        "genDType sqrt(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the value of which to take the square root.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return (genType:find(x) or genDType:find(x))
            and x or DataType.Undefined
    end,
})
