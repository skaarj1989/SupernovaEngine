return FunctionInfo({
    category = "Vector",
    name = "faceforward",
    description = "return a vector pointing in the same direction as another",
    signatures = {
        "genType faceforward(genType, genType, genType)",
        "genDType faceforward(genDType, genDType, genDType)",
    },
    args = {
        Parameter({
            name = "N",
            description = "Specifies the vector to orient.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "I",
            description = "Specifies the incident vector.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "Nref",
            description = "Specifies the reference vector.",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local N, I, Nref = args[1], args[2], args[3]
        if N == I and I == Nref then
            for _, group in pairs({ genType, genDType }) do
                if group:find(N) then
                    return N
                end
            end
        end
        return DataType.Undefined
    end,
})
