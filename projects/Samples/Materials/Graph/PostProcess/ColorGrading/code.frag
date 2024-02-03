const float id_16 = 0.000000;
const vec4 id_7 = texture(t_SceneColor, v_TexCoord);
const vec4 id_2 = colorGrade(id_7.xyz, t_LUT);
fragColor = id_2;
