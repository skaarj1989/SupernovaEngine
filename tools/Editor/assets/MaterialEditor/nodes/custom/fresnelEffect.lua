return FunctionInfo({
    category = "Common",
    name = "fresnelEffect",
    description = "",
    signature = "float fresnelEffect(vec3, vec3, float)",
    args = {
        Parameter({
            name = "N",
            description = "Specifies the normal vector.",
            attribute = Attribute.Normal,
        }),
        Parameter({
            name = "V",
            description = "Specifies the view direction vector.",
            constant = BuiltInConstant.ViewDir,
        }),
        Parameter({
            name = "power",
            description = "Specify the power.",
            value = 5.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local N, V, power = args[1], args[2], args[3]
        return (N == DataType.Vec3
                and V == DataType.Vec3
                and power == DataType.Float
            )
            and DataType.Float or DataType.Undefined
    end,
})
