return FunctionInfo({
    category = "Vector",
    name = "refract",
    description = "calculate the refraction direction for an incident vector",
    signatures = {
        "genType refract(genType, genType, float)",
        "genDType refract(genDType, genDType, float)",
    },
    args = {
        Parameter({
            name = "I",
            description = "Specifies the incident vector.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "N",
            description = "Specifies the normal vector.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "eta",
            description = "Specifies the ratio of indices of refraction.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local I, N, eta = args[1], args[2], args[3]
        if I == N and eta == DataType.Float then
            for _, group in pairs({ genType, genDType }) do
                if group:find(I) then
                    return I
                end
            end
        end
        return DataType.Undefined
    end,
})
