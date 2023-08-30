vec2 mytrunc(vec2 x, float numLevels) {
return floor(x * numLevels) / numLevels;
}
float mytrunc(float x, float numLevels) {
return floor(x * numLevels) / numLevels;
}
vec3 spectrumOffset(float t) {
float t0 = 3.0 * t - 1.5;return clamp01(vec3(-t0, 1.0 - abs(t0), t0));
}
vec4 glitch(sampler2D channel, vec2 uv, float strength) {
//uv.y = 1.0 - uv.y;

float time = mod(getTime(), 32.0);

float gnm = clamp01(strength);
float rnd0 = rand(mytrunc(vec2(time, time), 6.0));
float r0 = clamp01((1.0 - gnm) * 0.7 + rnd0);
float rnd1 = rand(vec2(mytrunc(uv.x, 10.0 * r0), time)); //horz
float r1 = 0.5 - 0.5 * gnm + rnd1;
r1 = 1.0 - max(0.0, ((r1 < 1.0) ? r1 : 0.9999999)); //note: weird ass bug on old drivers
float rnd2 = rand(vec2(mytrunc(uv.y, 40.0 * r1), time)); //vert
float r2 = clamp01(rnd2);

float rnd3 = rand(vec2(mytrunc(uv.y, 10.0 * r0), time));
float r3 = (1.0 - clamp01(rnd3 + 0.8)) - 0.1;

float pxrnd = rand(uv + time);

float ofs = 0.05 * r2 * strength * (rnd0 > 0.5 ? 1.0 : -1.0);
ofs += 0.5 * pxrnd * ofs;

uv.y += 0.1 * r3 * strength;

const int kNumSamples = 10;
const float kRcpNumSamples = 1.0 / float(kNumSamples);

vec4 sum = vec4(0.0);
vec3 wsum = vec3(0.0);
for (int i = 0; i < kNumSamples; ++i) {
	float t = float(i) * kRcpNumSamples;
	uv.x = clamp01(uv.x + ofs * t);
	vec4 samplecol = texture(channel, uv, -10.0);
	vec3 s = spectrumOffset(t);
	samplecol.rgb = samplecol.rgb * s;
	sum += samplecol;
	wsum += s;
}
sum.rgb /= wsum;
sum.a *= kRcpNumSamples;
return sum;
}
