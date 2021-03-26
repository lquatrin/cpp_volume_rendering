#version 430

layout(location = 0) in vec3 VerPos;

out vec2 color;
uniform mat4 ProjectionMatrix;

void main(void)
{
  gl_Position = ProjectionMatrix * vec4(VerPos,1.0);
  color = vec2((VerPos.x + 1.0) / 2.0, (VerPos.y + 1.0) / 2.0);
}