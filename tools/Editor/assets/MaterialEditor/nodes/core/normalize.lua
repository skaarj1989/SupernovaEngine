return FunctionInfo({
    category = "Vector",
    name = "normalize",
    description = "calculates the unit vector in the same direction as the original vector",
    signatures = {
        "genType normalize(genType)",
        "genDType normalize(genDType)",
    },
    args = {
        Parameter({
            name = "v",
            description = "Specifies the vector to normalize.",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local v = args[1]
        return (genType:find(v) or genDType:find(v))
            and v or DataType.Undefined
    end,
})
