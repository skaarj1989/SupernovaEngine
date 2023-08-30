return FunctionInfo({
    category = "Mathematics",
    name = "fract",
    description = "compute the fractional part of the argument",
    signatures = {
        "genType fract(genType)",
        "genDType fract(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the value to evaluate.",
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
