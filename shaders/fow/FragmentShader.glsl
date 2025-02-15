#version 330

// Input
in vec3 fcolor;
in float dist;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(mix(fcolor, fcolor / 100.f, min(15, dist) / 15.0f), 1);
}
