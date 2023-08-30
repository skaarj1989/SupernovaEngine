return FunctionInfo({
    category = nil,
    name = "getScreenCoord",
    description = "",
    signature = "vec4 getScreenCoord()",
    args = {},
    isEnabled = fragmentShaderOnly,
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 0)
        return DataType.Vec4
    end,
})
