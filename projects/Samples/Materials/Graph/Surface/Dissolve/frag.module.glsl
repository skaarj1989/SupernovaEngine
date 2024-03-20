float simpleNoise(vec2 P, vec2 step);

float simpleNoise(vec2 P, vec2 step) {
#define color(xy) cnoise(vec3(xy, 0))
float n = color(P);
n += 0.5 * color(P * 2.0 - step);
n += 0.25 * color(P * 4.0 - 2.0 * step);
n += 0.125 * color(P * 8.0 - 3.0 * step);
n += 0.0625 * color(P * 16.0 - 4.0 * step);
n += 0.03125 * color(P * 32.0 - 5.0 * step);
return n;
}
