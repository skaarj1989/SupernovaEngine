const float id_26 = 0.000000;
const vec4 id_17 = texture(t_albedo, getTexCoord0());
const vec3 id_15 = sRGBToLinear(id_17.xyz);
const float id_2 = 1.000000;
const float id_3 = 1.500000;
const float id_4 = 0.000000;
const float id_5 = 0.000000;
const vec3 id_6 = vec3(1, 1, 1);
const float id_7 = 1.000000;
const vec3 id_50 = sampleNormalMap(t_normal, getTexCoord0());
const vec3 id_53 = tangentToWorld(id_50, getNormal(), getTexCoord0());
const float id_37 = 0.000000;
const vec4 id_28 = texture(t_metallic, getTexCoord0());
const float id_48 = 0.000000;
const vec4 id_39 = texture(t_roughness, getTexCoord0());
const float id_11 = 1.000000;
const vec3 id_12 = vec3(0, 0, 0);
const float id_13 = 1.000000;
material.baseColor = id_15;
material.opacity = id_2;
material.ior = id_3;
material.transmissionFactor = id_4;
material.thickness = id_5;
material.attenuationColor = id_6;
material.attenuationDistance = id_7;
material.normal = id_53;
material.metallic = id_28.x;
material.roughness = id_39.x;
material.specular = id_11;
material.emissiveColor = id_12;
material.ambientOcclusion = id_13;
