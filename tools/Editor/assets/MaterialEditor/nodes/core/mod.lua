return FunctionInfo({
    category = "Mathematics",
    name = "mod",
    description = "compute value of one parameter modulo another",
    signatures = {
        "genType mod(genType, float)",
        "genType mod(genType, genType)",
        "genDType mod(genDType, double)",
        "genDType mod(genDType, genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specify the value to evaluate.",
            value = 0.0,
        }),
        Parameter({
            name = "y",
            description = "",
            value = 1.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local x, y = args[1], args[2]
        for _, bundle in pairs({ genTypeBundle, genDTypeBundle }) do
            if x == y and bundle.group:find(x) then
                -- gen*Type mod(gen*Type, gen*Type)
                return x
            elseif y == bundle.base and bundle.group:find(x) then
                -- gen*Type mod(gen*Type, base)
                return x
            end
        end
        return DataType.Undefined
    end,
})
