const vec2 texCoord = getTexCoord0();

material.baseColor.rgb = sRGBToLinear(texture(t_Base, texCoord).rgb);

material.metallic = 0.0;
material.roughness = 1.0;
