return FunctionInfo({
    category = "Mathematics",
    name = "max3",
    description = "",
    signature = "float max3(vec3)",
    args = {
        Parameter({
            name = "v",
            description = "value",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local v = args[1]
        return (v == DataType.Vec3)
            and DataType.Float or DataType.Undefined
    end,
})
