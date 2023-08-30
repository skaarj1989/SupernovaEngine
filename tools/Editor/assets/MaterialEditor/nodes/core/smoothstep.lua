return FunctionInfo({
    category = "Vector",
    name = "smoothstep",
    description = "perform Hermite interpolation between two values",
    signatures = {
        "genType smoothstep(genType, genType, genType)",
        "genType smoothstep(float, float, genType)",
        "genDType smoothstep(genDType, genDType, genDType)",
        "genDType smoothstep(double, double, genDType)",
    },
    args = {
        Parameter({
            name = "edge0",
            description = "Specifies the value of the lower edge of the Hermite function.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "edge1",
            description = "Specifies the value of the upper edge of the Hermite function.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "x",
            description = "Specifies the source value for interpolation.",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local edge0, edge1, x = args[1], args[2], args[3]
        for _, bundle in pairs({ genTypeBundle, genDTypeBundle }) do
            if edge0 == edge1 and edge1 == x and bundle.group:find(x) then
                -- gen*Type smoothstep(gen*Type, gen*Type, gen*Type)
                return x
            elseif edge0 == bundle.base and edge1 == bundle.base and bundle.group:find(x) then
                -- gen*Type smoothstep(base, base, gen*Type)
                return x
            end
        end
        return DataType.Undefined
    end,
})
