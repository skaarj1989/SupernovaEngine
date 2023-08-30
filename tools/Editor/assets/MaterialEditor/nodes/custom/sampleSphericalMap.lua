return FunctionInfo({
    category = "Texture",
    name = "sampleSphericalMap",
    description = "",
    signature = "vec2 sampleSphericalMap(vec3)",
    args = {
        Parameter({
            name = "dir",
            description = "Specifies the direction vector.",
            constant = BuiltInConstant.ViewDir,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local dir = args[1]
        return dir == DataType.Vec3 and DataType.Vec2 or DataType.Undefined
    end,
})
