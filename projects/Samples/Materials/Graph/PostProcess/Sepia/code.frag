const mat3 id_2 = mat3(
0.393, 0.349, 0.272,
0.769, 0.686, 0.534,
0.189, 0.168, 0.131
);
const float id_9 = 0.000000;
const vec4 id_6 = texture(t_SceneColor, v_TexCoord);
const vec3 id_3 = id_2 * id_6.xyz;
fragColor = vec4(id_3, 0);
