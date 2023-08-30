return FunctionInfo({
    category = "Vector",
    name = "dot",
    description = "calculate the dot product of two vectors",
    signatures = {
        "float dot(genType, genType)",
        "double dot(genDType, genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specifies the first of two vectors",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "y",
            description = "Specifies the second of two vectors",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local x, y = args[1], args[2]
        if x == y then
            local bundles = { genTypeBundle, genDTypeBundle }
            for _, bundle in pairs(bundles) do
                if bundle.group:find(x) then
                    return bundle.base
                end
            end
        end
        return DataType.Undefined
    end,
})
