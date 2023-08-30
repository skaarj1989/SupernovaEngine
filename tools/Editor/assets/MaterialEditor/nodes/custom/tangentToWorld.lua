return FunctionInfo({
    category = "Texture",
    name = "tangentToWorld",
    description = "",
    signature = "vec3 tangentToWorld(vec3, vec3, vec2)",
    args = {
        Parameter({
            name = "v",
            description = "",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "N",
            description = "Normal (if mesh does not have tangents)",
            attribute = Attribute.Normal,
        }),
        Parameter({
            name = "P",
            description = "TexCoord (if mesh does not have tangents)",
            attribute = Attribute.TexCoord0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local v, N, P = args[1], args[2], args[3]
        return (v == DataType.Vec3 and N == DataType.Vec3 and P == DataType.Vec2)
            and DataType.Vec3 or DataType.Undefined
    end,
})
