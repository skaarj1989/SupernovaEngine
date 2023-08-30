return FunctionInfo({
    category = "Vector",
    name = "cross",
    description = "calculate the cross product of two vectors",
    signatures = {
        "vec3 cross(vec3, vec3)",
        "dvec3 cross(dvec3, dvec3)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specifies the first of two vectors",
            value = math.vec3(1.0, 0.0, 0.0),
        }),
        Parameter({
            name = "y",
            description = "Specifies the second of two vectors",
            value = math.vec3(0.0, 1.0, 0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local x, y = args[1], args[2]
        return (x == y and x == DataType.Vec3 or x == DataType.DVec3)
            and x or DataType.Undefined
    end,
})
