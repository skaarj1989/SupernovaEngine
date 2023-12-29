#define _METALLIC 1 << 15
#define _ROUGHNESS 1 << 16
#define _AMBIENT_OCCLUSION 1 << 10

const vec2 texCoord = getTexCoord0();

#ifdef HAS_BASE_COLOR_TEXTURE
const vec4 baseColor = texture(t_BaseColor, texCoord);
material.baseColor.rgb = sRGBToLinear(baseColor.rgb);
#  ifdef HAS_BASE_COLOR_FACTOR
material.baseColor *= properties.baseColorFactor.rgb;
#  endif

#  if BLEND_MODE == BLEND_MODE_MASKED
#    if HAS_ALPHA_CUT_OFF
const float alphaCutOff = properties.alphaCutOff;
#    else
const float alphaCutOff = 0.7;
#    endif
material.opacity = step(alphaCutOff, baseColor.a);
#  elif BLEND_MODE == BLEND_MODE_TRANSPARENT
material.opacity = baseColor.a;
#  endif
#else // !defined HAS_BASE_COLOR_TEXTURE
#  ifdef HAS_BASE_COLOR_FACTOR
material.baseColor = properties.baseColorFactor.rgb;
#  endif
#endif

#ifdef HAS_NORMAL_TEXTURE
const vec3 N = sampleNormalMap(t_Normal, texCoord);
material.normal = tangentToWorld(N, getNormal(), texCoord);
#endif

material.roughness = 1.0;
material.metallic = 0.0;

#if IS_PBR
#  if PACKED_TEXTURE
const vec3 temp = texture(t_Packed, texCoord).rgb;
#    if PACKED_TEXTURE & _METALLIC
material.metallic = temp.b;
#    endif
#    if PACKED_TEXTURE & _ROUGHNESS
material.roughness = temp.g;
#    endif
#    if PACKED_TEXTURE & _AMBIENT_OCCLUSION
material.ambientOcclusion = temp.r;
#    endif
#  endif

#  ifdef HAS_METALLIC_FACTOR
material.metallic *= properties.metallicFactor;
#  endif
#  ifdef HAS_ROUGHNESS_FACTOR
material.roughness *= properties.roughnessFactor;
#  endif

#  ifdef HAS_AMBIENT_OCCLUSION_TEXTURE
material.ambientOcclusion = texture(t_AmbientOcclusion, texCoord).r;
#  endif

#  ifdef HAS_TRANSMISSION_TEXTURE
material.transmissionFactor = texture(t_Transmission, texCoord).r;
#    ifdef HAS_TRANSMISSION_FACTOR
material.transmissionFactor *= properties.transmissionFactor;
#    endif
#  else // !defined HAS_TRANSMISSION_TEXTURE
#    ifdef HAS_TRANSMISSION_FACTOR
material.transmissionFactor = properties.transmissionFactor;
#    endif
#  endif

#  ifdef HAS_THICKNESS_TEXTURE
material.thickness = texture(t_Thickness, texCoord).g;
#    ifdef HAS_THICKNESS_FACTOR
material.thickness *= properties.thicknessFactor;
#    endif
#  else // !defined HAS_THICKNESS_TEXTURE
#    ifdef HAS_THICKNESS_FACTOR
material.thickness = properties.thicknessFactor;
#    endif
#  endif

#  ifdef HAS_ATTENUATION_COLOR
material.attenuationColor = properties.attenuationColor.rgb;
#  endif
#  ifdef HAS_ATTENUATION_DISTANCE
material.attenuationDistance = properties.attenuationDistance;
#  endif
#endif // defined IS_PBR

#ifdef HAS_EMISSIVE_COLOR_TEXTURE
material.emissiveColor.rgb = texture(t_EmissiveColor, texCoord).rgb;
#  ifdef HAS_EMISSIVE_COLOR_FACTOR
material.emissiveColor *= properties.emissiveColorFactor.rgb;
#  endif
#else // !defined HAS_EMISSIVE_COLOR_TEXTURE
#  ifdef HAS_EMISSIVE_COLOR_FACTOR
material.emissiveColor = properties.emissiveColorFactor.rgb;
#  endif
#endif
