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

#if IS_PBR
#  ifdef HAS_METALLIC_ROUGHNESS_A_O_TEXTURE
const vec3 temp = texture(t_MetallicRoughnessAO, texCoord).rgb;
material.metallic = temp.b;
material.roughness = temp.g;
material.ambientOcclusion = temp.r;
#  endif
#  ifdef HAS_METALLIC_FACTOR
material.metallic *= properties.metallicFactor;
#  endif
#  ifdef HAS_ROUGHNESS_FACTOR
material.roughness *= properties.roughnessFactor;
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
#else
material.roughness = 1.0;
material.metallic = 0.0;
#endif

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
