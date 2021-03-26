#version 430

layout (location = 0) out vec4 FragColor;

in vec2 color;

uniform sampler2D TexGeneratedFrame;

void main (void)
{
  vec4 frag_color = texture(TexGeneratedFrame, color);
  //float gamma = 2.2;
  //frag_color.rgb = pow(frag_color.rgb, vec3(1.0/gamma));

  FragColor = frag_color;
}