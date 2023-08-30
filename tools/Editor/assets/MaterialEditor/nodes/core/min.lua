return FunctionInfo({
    category = "Mathematics",
    name = "min",
    description = "return the lesser of two values",
    signatures = {
        "genType min(genType, genType)",
        "genType min(genType, float)",
        "genDType min(genDType, genDType)",
        "genDType min(genDType, double)",
        "genIType min(genIType, genIType)",
        "genIType min(genIType, int)",
        "genUType min(genUType, genUType)",
        "genUType min(genUType, uint)",
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
                -- gen*Type min(gen*Type, gen*Type)
                return x
            elseif y == bundle.base and bundle.group:find(x) then
                -- gen*Type min(gen*Type, base)
                return x
            end
        end
        return DataType.Undefined
    end,
})
