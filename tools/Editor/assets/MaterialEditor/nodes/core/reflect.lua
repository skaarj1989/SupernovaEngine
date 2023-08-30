return FunctionInfo({
    category = "Vector",
    name = "reflect",
    description = "calculate the reflection direction for an incident vector",
    signatures = {
        "genType reflect(genType, genType)",
        "genDType reflect(genDType, genDType)",
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
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local I, N = args[1], args[2]
        if I == N then
            for _, group in pairs({ genType, genDType }) do
                if group:find(I) then
                    return I
                end
            end
        end
        return DataType.Undefined
    end,
})
