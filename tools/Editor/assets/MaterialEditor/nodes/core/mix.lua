return FunctionInfo({
    category = "Mathematics",
    name = "mix",
    description = "linearly interpolate between two values",
    signatures = {
        "genType mix(genType, genType, genType)",
        "genType mix(genType, genType, float)",
        "genDType mix(genDType, genDType, genDType)",
        "genDType mix(genDType, genDType, double)",
        "genType mix(genType, genType, genBType)",
        "genDType mix(genDType, genDType, genBType)",
        "genIType mix(genIType, genIType, genBType)",
        "genUType mix(genUType, genUType, genBType)",
        "genBType mix(genBType, genBType, genBType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the start of the range in which to interpolate.",
            value = 0.0,
        }),
        Parameter({
            name = "y",
            description = "Specify the end of the range in which to interpolate.",
            value = 0.0,
        }),
        Parameter({
            name = "a",
            description = "Specify the value to use to interpolate between x and y.",
            value = 1.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local x, y, a = args[1], args[2], args[3]
        for _, bundle in pairs({ genTypeBundle, genDTypeBundle }) do
            if x == y and y == a and bundle.group:find(x) then
                -- gen*Type mix(gen*Type, gen*Type, gen*Type)
                return x
            elseif x == y and a == bundle.base and bundle.group:find(x) then
                -- gen*Type mix(gen*Type, gen*Type, base)
                return x
            end
        end
        return DataType.Undefined
    end,
})
