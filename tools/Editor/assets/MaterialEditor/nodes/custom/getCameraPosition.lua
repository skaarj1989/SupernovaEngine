return FunctionInfo({
    category = nil,
    name = "getCameraPosition",
    description = "",
    signature = "vec3 getCameraPosition()",
    args = {},
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 0)
        return DataType.Vec3
    end,
})
