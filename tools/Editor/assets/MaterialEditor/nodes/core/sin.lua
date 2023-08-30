return FunctionInfo({
    category = "Trigonometry",
    name = "sin",
    description = "return the sine of the parameter",
    signature = "genType sin(genType)",
    args = {
        Parameter({
            name = "angle",
            description = "Specify the quantity, in radians, of which to return the sine.",
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
