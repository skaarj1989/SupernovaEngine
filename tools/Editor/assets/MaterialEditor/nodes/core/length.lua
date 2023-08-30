return FunctionInfo({
    category = "Vector",
    name = "length",
    description = "calculate the length of a vector",
    signatures = {
        "float length(genType)",
        "double length(genDType)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specifies a vector of which to calculate the length.",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        for _, bundle in pairs({ genTypeBundle, genDTypeBundle }) do
            if bundle.group:find(x) then
                return bundle.base
            end
        end
        return DataType.Undefined
    end,
})
