return FunctionInfo({
    category = nil,
    name = "srand",
    description = "[-1..1]",
    signature = "float srand(vec2)",
    args = {
        Parameter({
            name = "co",
            description = "",
            attribute = Attribute.TexCoord0,
        })
    },
    getReturnType = function(args)
        assert(#args == 1)
        local co = args[1]
        return co == DataType.Vec2 and DataType.Float or DataType.Undefined
    end,
})
