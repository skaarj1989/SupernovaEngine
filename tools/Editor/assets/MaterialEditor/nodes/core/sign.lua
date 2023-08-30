return FunctionInfo({
    category = "Mathematics",
    name = "sign",
    description = "extract the sign of the parameter",
    signatures = {
        "genType sign(genType)",
        "genIType sign(genIType)",
        "genDType sign(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the value from which to extract the sign.",
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
