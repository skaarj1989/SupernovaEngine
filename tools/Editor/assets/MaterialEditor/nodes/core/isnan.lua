return FunctionInfo({
    category = "Mathematics",
    name = "isnan",
    description = "determine whether the parameter is a number",
    signatures = {
        "genBType isnan(genType)",
        "genBType isnan(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specifies the value to test for NaN.",
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
