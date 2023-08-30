return FunctionInfo({
    category = "Trigonometry",
    name = "sinh",
    description = "return the hyperbolic sine of the parameter",
    signature = "genType sinh(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value whose hyperbolic sine to return.",
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
