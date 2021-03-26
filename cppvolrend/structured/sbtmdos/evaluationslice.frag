#version 430 compatibility

layout (location = 0) out vec4 EyeBuffer;
layout (location = 1) out vec4 OcclusionBuffer;

in vec3 world_pos;
in vec2 CVector;
in vec4 pos_clip_space;

uniform sampler3D TexVolume;
uniform sampler1D TexTransferFunc;

uniform vec3 VolumeScaledSizes;

uniform float StepSize;

uniform vec2 ScreenSize;

uniform sampler2D EyeBufferPrev;
uniform sampler2D OcclusionBufferPrev;

uniform float OcclusionExtent; 
uniform vec2 GridSize;

uniform float VolumeDiagonal;

uniform float UITransparencyScale;

vec4 GetFromTransferFunction (vec3 in_pos)
{
  float vol_density = texture(TexVolume, in_pos / VolumeScaledSizes).r;
  return texture(TexTransferFunc, vol_density);
}

float Algorithm3 (float st, float slice_distance)
{
  //////////////////////////////
  // Equation 13
  vec2 xy_d = pos_clip_space.xy / pos_clip_space.w;
  
  float s_occlusion = 0.0;

  // x of y, they're equal
  float x_grid_pos = 0.5;
  int xpos_sample = 0;
  while(xpos_sample < GridSize.x)
  {
    float y_grid_pos = 0.5;
    int ypos_sample = 0;
    while(ypos_sample < GridSize.y)
    {
      //////////////////////////////
      // Generate each sample on an uniform grid an multiply by the current radius to evaluate occlusion
      // . center ir 0.0
      // ***********
      // * X  X  X *
      // * X  X  X *
      // * X  X  X *
      // ***********
      vec2 p = ((vec2(x_grid_pos, y_grid_pos) - (GridSize / 2.0f)) / (GridSize / 2.0f)) * OcclusionExtent;
      
      //////////////////////////////
      // Equation 14
      // must normalize the grid pos to multiply by the radius of the occlusion extent
      vec2 p_d = xy_d + CVector * p;

      /////////////////////////////////
      // Equation 15
      vec2 p_t = 0.5 * p_d + vec2(0.5);

      float occ_sample = texture(OcclusionBufferPrev, p_t).r;
      s_occlusion += occ_sample;
  
      y_grid_pos += 1.0f;
      ypos_sample = ypos_sample + 1;
    }
    x_grid_pos += 1.0f;
    xpos_sample = xpos_sample + 1;
  }
  s_occlusion = (s_occlusion / (GridSize.x * GridSize.y)) * exp(-st * slice_distance * UITransparencyScale);

  return s_occlusion;
}

// http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch39.html
void main(void)
{
  vec3 aabb = VolumeScaledSizes * 0.5;
  if (world_pos.x > aabb.x || world_pos.x < -aabb.x  ||
      world_pos.y > aabb.y || world_pos.y < -aabb.y  ||
      world_pos.z > aabb.z || world_pos.z < -aabb.z )
    discard;
  else
  {
    vec3 pos_from_zero = world_pos + aabb;
    vec4 src = GetFromTransferFunction(pos_from_zero);
 
    float st = src.a;

    vec4 dst = texelFetch(EyeBufferPrev, ivec2(gl_FragCoord.st), 0);

    float occlusion;
    //if (!(dst.a > 0.99))
    {
      occlusion = texture(OcclusionBufferPrev, (gl_FragCoord.st) / ScreenSize).r;
     
      // Evaluate the current opacity
      src.a = 1.0 - exp(-src.a * StepSize);
      
      // Front-to-back composition
      src.rgb = src.rgb * occlusion * src.a;
      
      dst = dst + (1.0 - dst.a) * src;
    }
    occlusion = Algorithm3(st, StepSize);
    OcclusionBuffer = vec4(occlusion, 0, 0, 0);

    EyeBuffer = dst;
  }
}