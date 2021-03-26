#version 430

layout(location = 0) in vec3 VerPos;

void main (void)
{
	gl_Position = vec4(VerPos, 1.0);
}