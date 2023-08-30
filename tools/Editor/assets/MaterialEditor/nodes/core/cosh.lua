return FunctionInfo({
    category = "Trigonometry",
    name = "cosh",
    description = "return the hyperbolic cosine of the parameter",
    signature = "genType cosh(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value whose hyperbolic cosine to return.",
            value = 0.0,
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return genType:find(x) and x or DataType.Undefined
    end,
})
