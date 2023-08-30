return FunctionInfo({
    category = "Mathematics",
    name = "sq",
    description = "x^2",
    signature = "genType sq(genType)",
    args = {
        Parameter({
            name = "x",
            description = "",
            value = 0.0,
        })
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return genType:find(x) and x or DataType.Undefined
    end,
})
