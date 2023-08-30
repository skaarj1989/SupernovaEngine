const mat3 id_13 = mat3(
0.393, 0.349, 0.272,
0.769, 0.686, 0.534,
0.189, 0.168, 0.131
);
const float id_12 = 0.000000;
const vec4 id_3 = texture(t_SceneColor, v_TexCoord);
const vec3 id_14 = id_13 * id_3.xyz;
fragColor = vec4(id_14, 0);
