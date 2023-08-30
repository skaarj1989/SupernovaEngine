return FunctionInfo({
    category = "Mathematics",
    name = "max",
    description = "return the greater of two values",
    signatures = {
        "genType max(genType, genType)",
        "genType max(genType, float)",
        "genDType max(genDType, genDType)",
        "genDType max(genDType, double)",
        "genIType max(genIType, genIType)",
        "genIType max(genIType, int)",
        "genUType max(genUType, genUType)",
        "genUType max(genUType, uint)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the first value to compare.",
            value = 0.0,
        }),
        Parameter({
            name = "y",
            description = "Specify the second value to compare.",
            value = 1.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local x, y = args[1], args[2]
        local bundles = {
            genTypeBundle,
            genDTypeBundle,
            genITypeBundle,
            genUTypeBundle,
        }
        for _, bundle in pairs(bundles) do
            if x == y and bundle.group:find(x) then
                -- gen*Type(gen*Type, gen*Type)
                return x
            elseif y == bundle.base and bundle.group:find(x) then
                -- gen*Type(gen*Type, base)
                return x
            end
        end
        return DataType.Undefined
    end,
})
