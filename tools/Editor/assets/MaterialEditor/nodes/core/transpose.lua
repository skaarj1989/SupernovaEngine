return FunctionInfo({
    category = "Matrix",
    name = "transpose",
    description = "calculate the transpose of a matrix",
    signatures = {
        "mat2 transpose(mat2)",
        "mat3 transpose(mat3)",
        "mat4 transpose(mat4)",
    },
    args = {
        Parameter({
            name = "m",
            description = "Specifies the matrix of which to take the transpose.",
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local m = args[1]
        return isMatrix(m) and m or DataType.Undefined
    end,
})
