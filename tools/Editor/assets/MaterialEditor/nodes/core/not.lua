return FunctionInfo({
    category = "Component Comparison",
    name = "not",
    description = "logically invert a boolean vector",
    signature = "bvec not(bvec)",
    args = {
        Parameter({
            name = "x",
            description = "Specifies the vector to be inverted.",
            value = math.bvec3(false),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return bvec:find(x) and x or DataType.Undefined
    end,
})
