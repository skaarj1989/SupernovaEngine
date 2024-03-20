vec4 colorGrade(vec3 color, sampler2D lut);

vec4 colorGrade(vec3 color, sampler2D lut) {
const float kNumColors = 16.0;
const float kMaxCellIdx = kNumColors - 1.0;
const float kThreshold = kMaxCellIdx / kNumColors;

const float cellIdx = color.b * kMaxCellIdx;
const vec2 cell = vec2(floor(cellIdx), ceil(cellIdx));

const vec2 lutSize = vec2(textureSize(lut, 0));
const vec2 halfPx = vec2(0.5) / lutSize;

const vec2 offset = halfPx +
  vec2(
    color.r * kThreshold / kNumColors,
    color.g * kThreshold
  );
const vec2 lutPos = cell / kNumColors + offset.r;

const vec4 gradedColorL = textureLod(lut, vec2(lutPos.r, offset.g), 0);
const vec4 gradedColorR = textureLod(lut, vec2(lutPos.g, offset.g), 0);
return mix(gradedColorL, gradedColorR, fract(cellIdx));
}
