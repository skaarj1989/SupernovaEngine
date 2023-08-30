return FunctionInfo({
    category = nil,
    name = "billboard",
    description = "",
    signature = "vec4 billobard(mat4, vec4, bool)",
    args = {
        Parameter({
            name = "m",
            description = "ModelMatrix",
            constant = BuiltInConstant.ModelMatrix,
        }),
        Parameter({
            name = "P",
            description = "Position",
            attribute = Attribute.Position,
        }),
        Parameter({
            name = "spherical",
            description = "Spherical or Cylindrical",
            value = true,
        }),
    },
    --- @param shaderType ShaderType
    --- @param blueprint MaterialBlueprint
    isEnabled = function(shaderType, blueprint)
        return shaderType == ShaderType.Vertex
            and getDomain(blueprint) == MaterialDomain.Surface
    end,
    --- @param args DataType[]
    getReturnType = function(args)
        assert(#args == 3)
        local m, P, spherical = args[1], args[2], args[3]
        return (m == DataType.Mat4
                and P == DataType.Vec4
                and spherical == DataType.Bool
            )
            and DataType.Vec4 or DataType.Undefined
    end,
})
