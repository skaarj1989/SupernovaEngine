#version 460 core

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 FragColor;
void main() {
  FragColor = vec4(1.0, 0.0, 1.0, 1.0);
  // https://github.com/KhronosGroup/glTF-Sample-Viewer/commit/9da7b3aca927a57a0abe4b9a32d565c6d72e5a4d
  FragColor.rb =
    vec2(max(2.0 * sin(0.1 * (gl_FragCoord.x + gl_FragCoord.y)), 0.0) + 0.3);
}
