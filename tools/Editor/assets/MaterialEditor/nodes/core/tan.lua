return FunctionInfo({
    category = "Trigonometry",
    name = "tan",
    description = "return the tangent of the parameter",
    signature = "genType tan(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the quantity, in radians, of which to return the tangent.",
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
