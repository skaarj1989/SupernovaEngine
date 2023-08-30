return FunctionInfo({
    category = "Vector",
    name = "equal",
    description = "perform a component-wise equal-to comparison of two vectors",
    signatures = {
        "bvec equal(vec, vec)",
        "bvec equal(ivec, ivec)",
        "bvec equal(uvec, uvec)",
    },
    args = {
        Parameter({
            name = "x",
            description = "Specifies the first vector to be used in the comparison operation.",
            value = math.vec3(0.0),
        }),
        Parameter({
            name = "y",
            description = "Specifies the second vector to be used in the comparison operation.",
            value = math.vec3(0.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 2)
        local x, y = args[1], args[2]
        if x == y then
            for _, group in pairs({ vec, ivec, uvec }) do
                if group:find(x) then
                    return constructVectorType(DataType.Bool, countChannels(x))
                end
            end
        end
        return DataType.Undefined
    end,
})
