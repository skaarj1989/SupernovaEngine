const vec2 texCoord = getTexCoord0();

const vec4 baseColor = texture(t_BaseColor, texCoord);
material.baseColor.rgb = sRGBToLinear(baseColor.rgb);

const vec3 N = sampleNormalMap(t_Normal, texCoord);
material.normal = tangentToWorld(N, getNormal(), texCoord);

const vec3 temp = texture(t_MetallicRoughnessAO, texCoord).rgb;
material.metallic = temp.b;
material.roughness = temp.g;
material.ambientOcclusion = texture(t_AO, texCoord).r;

material.emissiveColor.rgb = texture(t_EmissiveColor, texCoord).rgb;
