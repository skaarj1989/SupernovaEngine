return FunctionInfo({
    category = "Mathematics",
    name = "isinf",
    description = "determine whether the parameter is positive or negative infinity",
    signatures = {
        "genBType isinf(genType)",
        "genBType isinf(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specifies the value to test for infinity.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return (genType:find(x) or genDType:find(x))
            and constructVectorType(DataType.Bool, countChannels(x)) or DataType.Undefined
    end,
})
