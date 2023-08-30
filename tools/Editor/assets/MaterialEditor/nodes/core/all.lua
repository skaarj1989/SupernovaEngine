return FunctionInfo({
    category = "Component Comparison",
    name = "all",
    description = "check whether all elements of a boolean vector are true",
    signature = "bool all(bvec)",
    args = {
        Parameter({
            name = "x",
            description = "Specifies the vector to be tested for truth.",
            value = math.bvec3(false),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 1)
        local x = args[1]
        return bvec:find(x) and DataType.Bool or DataType.Undefined
    end,
})
