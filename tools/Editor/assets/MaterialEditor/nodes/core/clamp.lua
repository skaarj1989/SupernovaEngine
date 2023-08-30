return FunctionInfo({
    category = "Mathematics",
    name = "clamp",
    description = "constrain a value to lie between two further values",
    signatures = {
        "genType clamp(genType, genType, genType)",
        "genType clamp(genType, float, float)",
        "genDType clamp(genDType, genDType, genDType)",
        "genDType clamp(genDType, double, double)",
        "genIType clamp(genIType, genIType, genIType)",
        "genIType clamp(genIType, int, int)",
        "genUType clamp(genUType, genUType, genUType)",
        "genUType clamp(genUType, uint, uint)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the value to constrain.",
            value = 0.0,
        }),
        Parameter({
            name = "minVal",
            description = "Specify the lower end of the range into which to constrain x.",
            value = 0.0,
        }),
        Parameter({
            name = "maxVal",
            description = "Specify the upper end of the range into which to constrain x.",
            value = 1.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local x, minVal, maxVal = args[1], args[2], args[3]
        local bundles = {
            genTypeBundle,
            genDTypeBundle,
            genITypeBundle,
            genUTypeBundle,
        }
        for _, bundle in pairs(bundles) do
            if x == bundle.base and minVal == x and maxVal == minVal then
                -- gen*Type clamp(gen*Type, gen*Type, gen*Type)
                return x
            elseif minVal == bundle.base and maxVal == bundle.base and bundle.group:find(x) then
                -- gen*Type clamp(gen*Type, base, base)
                return x
            end
        end
        return DataType.Undefined
    end,
})
