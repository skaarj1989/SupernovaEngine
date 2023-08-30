return FunctionInfo({
    category = "Trigonometry",
    name = "degrees",
    description = "convert a quantity in radians to degrees",
    signature = "genType degrees(genType)",
    args = {
        Parameter({
            name = "radians",
            description = "Specify the quantity, in radians, to be converted to degrees.",
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
