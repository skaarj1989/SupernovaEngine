return FunctionInfo({
    category = "Matrix",
    name = "inverse",
    description = "calculate the transpose of a matrix",
    signatures = {
        "mat2 inverse(mat2)",
        "mat3 inverse(mat3)",
        "mat4 inverse(mat4)",
    },
    args = {
        Parameter({
            name = "m",
            description = "Specifies the matrix of which to take the inverse.",
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local m = args[1]
        return isMatrix(m) and m or DataType.Undefined
    end,
})
