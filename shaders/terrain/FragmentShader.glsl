#version 330

// Input
in float noise;
in float dist;

// Output
layout(location = 0) out vec4 out_color;

// Variables
uniform int fow;

vec3 color_green = vec3(0.0, 0.392, 0.0);
vec3 color_brown = vec3(0.545, 0.271, 0.0);

void main()
{
	vec3 tmp = mix(color_green, color_brown, noise);
	if (fow > 0){
		tmp = mix(tmp, tmp / 100.f, min(dist, 15) / 15.f);
	}

	out_color = vec4(tmp, 1);
}
