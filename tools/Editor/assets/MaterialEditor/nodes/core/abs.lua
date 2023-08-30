return FunctionInfo({
    category = "Mathematics",
    name = "abs",
    description = "return the absolute value of the parameter",
    signatures = {
        "genType abs(genType)",
        "genIType abs(genIType)",
        "genDType abs(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the value of which to return the absolute.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        for _, group in pairs({ genType, genIType, genDType }) do
            if group:find(x) then
                return x
            end
        end
        return DataType.Undefined
    end,
})
