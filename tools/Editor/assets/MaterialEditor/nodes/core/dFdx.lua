return FunctionInfo({
    category = "Mathematics",
    name = "dFdx",
    description = "return the partial derivative of an argument with respect to x",
    signature = "genType dFdx(genType)",
    args = {
        Parameter({
            name = "p",
            description = "Specifies the expression of which to take the partial derivative.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local p = args[1]
        return genType:find(p) and p or DataType.Undefined
    end,
})
