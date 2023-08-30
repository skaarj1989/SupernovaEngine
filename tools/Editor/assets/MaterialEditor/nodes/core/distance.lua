return FunctionInfo({
    category = "Vector",
    name = "distance",
    description = "calculate the distance between two points",
    signatures = {
        "float distance(genType, genType)",
        "double distance(genDType, genDType)",
    },
    args = {
        Parameter({
            name = "p0",
            description = "Specifies the first of two points",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "p1",
            description = "Specifies the second of two points",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local p0, p1 = args[1], args[2]
        if p0 == p1 then
            for _, bundle in pairs({ genTypeBundle, genDTypeBundle }) do
                if bundle.group:find(p0) then
                    return bundle.base
                end
            end
        end
        return DataType.Undefined
    end,
})
