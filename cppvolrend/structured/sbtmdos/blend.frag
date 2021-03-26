#version 430 compatibility

layout (location = 0) out vec4 FragColor;

uniform sampler2D EyeBufferSlice;

uniform vec2 ScreenSize;

void main(void)
{
  FragColor = texture(EyeBufferSlice, gl_FragCoord.st / ScreenSize);
}