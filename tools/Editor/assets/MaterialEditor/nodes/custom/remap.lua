return FunctionInfo({
    category = "Mathematics",
    name = "remap",
    description = "",
    signatures = {
        "float remap(float, vec2, vec2)",
        "vec2 remap(vec2, vec2, vec2)",
        "vec3 remap(vec3, vec2, vec2)",
        "vec4 remap(vec4, vec2, vec2)",
    },
    args = {
        Parameter({
            name = "v",
            description = "value",
            value = 0.0,
        }),
        Parameter({
            name = "i",
            description = "input range",
            value = math.vec2(0.0, 1.0),
        }),
        Parameter({
            name = "o",
            description = "output range",
            value = math.vec2(-1.0, 1.0),
        }),
    },
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local v, i, o = args[1], args[2], args[3]
        return (i == DataType.Vec2 and o == i and genType:find(v))
            and v or DataType.Undefined
    end,
})
