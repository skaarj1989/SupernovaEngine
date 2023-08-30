return FunctionInfo({
    category = "Mathematics",
    name = "step",
    description = "generate a step function by comparing two values",
    signatures = {
        "genType step(genType, genType)",
        "genType step(float, genType)",
        "genDType step(genDType, genDType)",
        "genDType step(double, genDType)",
    },
    args = {
        Parameter({
            name = "edge",
            description = "Specifies the location of the edge of the step function.",
            value = 0.0,
        }),
        Parameter({
            name = "x",
            description = "Specify the value to be used to generate the step function.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local edge, x = args[1], args[2]
        for _, bundle in pairs({ genTypeBundle, genDTypeBundle }) do
            if x == edge then
                -- gen*Type step(gen*Type, gen*Type)
                return x
            elseif edge == bundle.base and bundle.group:find(x) then
                -- gen*Type step(base, gen*Type)
                return x
            end
        end
        return DataType.Undefined
    end,
})
