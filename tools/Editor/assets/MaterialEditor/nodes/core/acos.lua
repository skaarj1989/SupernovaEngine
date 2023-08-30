return FunctionInfo({
    category = "Trigonometry",
    name = "acos",
    description = "return the arccosine of the parameter",
    signature = "genType acos(genType)",
    args = {
        Parameter({
            name = "x",
            description = "Specify the value whose arccosine to return.",
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
