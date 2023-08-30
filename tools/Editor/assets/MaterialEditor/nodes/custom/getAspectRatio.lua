return FunctionInfo({
    category = nil,
    name = "getAspectRatio",
    description = "",
    signature = "float getAspectRatio()",
    args = {},
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 0)
        return DataType.Float
    end,
})
