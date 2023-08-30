return FunctionInfo({
    category = "Trigonometry",
    name = "asinh",
    description = "return the arc hyperbolic sine of the parameter",
    signature = "genType asinh(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value whose arc hyperbolic sine to return.",
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
