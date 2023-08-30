return FunctionInfo({
    category = "Mathematics",
    name = "fma",
    description = "perform a fused multiply-add operation",
    signatures = {
        "genType fma(genType, genType, genType)",
        "genDType fma(genDType, genDType, genDType)",
    },
    args = {
        Parameter({
            name = "a",
            description = "Specifies the first multiplicand.",
            value = 0.0,
        }),
        Parameter({
            name = "b",
            description = "Specifies the second multiplicand.",
            value = 0.5,
        }),
        Parameter({
            name = "c",
            description = "Specifies the value to be added to the result.",
            value = 0.5,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local a, b, c = args[1], args[2], args[3]
        if a == b and b == c then
            for _, group in pairs({ genType, genDType }) do
                if group:find(a) then
                    return a
                end
            end
        end
        return DataType.Undefined
    end,
})
