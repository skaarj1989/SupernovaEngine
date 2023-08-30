return FunctionInfo({
    category = "Matrix",
    name = "determinant",
    description = "calculate the determinant of a matrix",
    signatures = {
        "float determinant(mat2)",
        "float determinant(mat3)",
        "float determinant(mat4)",
    },
    args = {
        Parameter({
            name = "m",
            description = "Specifies the matrix of which to take the determinant.",
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local m = args[1]
        return isMatrix(m) and DataType.Float or DataType.Undefined
    end,
})
