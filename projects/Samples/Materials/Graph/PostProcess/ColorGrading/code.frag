const float id_8 = 0.000000;
const vec4 id_5 = texture(t_SceneColor, v_TexCoord);
const vec4 id_2 = colorGrade(id_5.xyz, t_fs_LUT);
fragColor = id_2;
