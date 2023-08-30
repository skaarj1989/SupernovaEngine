return FunctionInfo({
    category = "Trigonometry",
    name = "acosh",
    description = "return the arc hyperbolic cosine of the parameter",
    signature = "genType acosh(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value whose arc hyperbolic cosine to return.",
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
