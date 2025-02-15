#version 330

// Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform float time;

// Output
uniform vec3 dronePos;

out float noise;
out float dist;

// Variables
float random(vec2 st) {
	return fract(sin(st.x) + cos(st.y));
}

float makeNoise(vec2 coord) {
	vec2 i = floor(coord);
	vec2 f = fract(coord);

	// Four corners in 2D of a tile
	float a = random(i + sin(time));
	float b = random(i + sin(time) + vec2(1.0f, 0.0f));
	float c = random(i + sin(time) + vec2(0.0f, 1.0f));
	float d = random(i + sin(time) + vec2(1.0f, 1.0f));

	// Smooth Interpolation

	// Cubic Hermine Curve.  Same as SmoothStep()
	vec2 u = f * f * (3.0f - 2.0f * f);
	// vec2 u = smoothstep(0.0f , 1.0f , f);

	// Mix 4 corners percentages
	return mix(a, b, u.x) +
			(c - a)* u.y * (1.0f - u.x) +
			(d - b) * u.x * u.y;
}

void main()
{
	vec3 worldPos = (Model * vec4(pos, 1.0f)).xyz;

	noise = makeNoise(vec2(worldPos.x, worldPos.z) * 0.5f);
	vec3 newPos = pos;

	newPos.y = mix(0.0f, 0.5f, 1.0f - noise);
	dist = distance(worldPos, dronePos);

	gl_Position = Projection * View * Model * vec4(newPos, 1);
}
