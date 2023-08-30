return FunctionInfo({
    category = "Color",
    name = "getLuminance",
    description = "",
    signature = "float getLuminance(vec3)",
    args = {
        Parameter({
            name = "color",
            description = "",
            value = math.vec3(0.0),
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local color = args[1]
        return (color == DataType.Vec3)
            and DataType.Float or DataType.Undefined
    end,
})
