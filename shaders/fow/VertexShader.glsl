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

// Output
uniform vec3 dronePos;

out vec3 fcolor;
out float dist;

void main()
{
	vec3 worldPos = (Model * vec4(pos, 1.0f)).xyz;
	dist = distance(worldPos, dronePos);

	fcolor = color;

	gl_Position = Projection * View * Model * vec4(pos, 1);
}
