return FunctionInfo({
    category = "Trigonometry",
    name = "cos",
    description = "return the cosine of the parameter",
    signature = "genType cos(genType)",
    args = {
        Parameter({
            name = "angle",
            description = "Specify the quantity, in radians, of which to return the cosine.",
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
