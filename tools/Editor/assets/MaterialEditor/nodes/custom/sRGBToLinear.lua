return FunctionInfo({
    category = "Color",
    name = "sRGBToLinear",
    description = "",
    signatures = {
        "vec3 sRGBToLinear(vec3)",
        "vec4 sRGBToLinear(vec4)",
    },
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
        return (color == DataType.Vec3 or color == DataType.Vec4)
            and color or DataType.Undefined
    end,
})
