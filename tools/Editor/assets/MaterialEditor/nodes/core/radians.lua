return FunctionInfo({
    category = "Trigonometry",
    name = "radians",
    description = "convert a quantity in degrees to radians",
    signature = "genType radians(genType)",
    args = {
        Parameter({
            name = "degrees",
            description = "Specify the quantity, in degrees, to be converted to radians.",
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
