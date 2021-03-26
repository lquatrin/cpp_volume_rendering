#version 430

in vec3 vert_eye;
in vec3 vert_dir;

layout (location = 0) out vec4 FragColor;

uniform sampler3D TexSuperVoxelsVolume;
uniform sampler2D TexPreIntegrationLookup;

uniform vec3 WorldLightingPos;
uniform vec3 WorldEyePos;

uniform vec3 VolumeScaledSizes;
uniform int ApplyPhongShading;

uniform float StepSize;

uniform int ApplyOcclusion;
uniform int ApplyShadow;

 // Cone Tracing Sampling Rate
uniform float ConeStepSize;
uniform float ConeStepIncreaseRate;
uniform float ConeInitialStep;

// Cone Aperture Apex Angle - Shadow softness
uniform float TanRadiusConeApexAngle;

// Opacity Correction Factor - Shadow darkness
uniform int ApplyOpacityCorrectionFactor;
uniform float OpacityCorrectionFactor;

// Cone Number of Samples
uniform int ConeNumberOfSamples;

uniform float VolumeMaxDensity;
uniform float VolumeMaxStandardDeviation;

// Gradient-based shading
uniform sampler3D TexVolumeGradient;
uniform float Kambient;
uniform float Kdiffuse;
uniform float Kspecular;
uniform vec3 Ispecular;
uniform float Nshininess;

//////////////////////////////////////////////////////////////////////////////////////////////////
// From common/raybboxintersection.frag
struct Ray { vec3 Origin; vec3 Dir; };
bool RayAABBIntersection (vec3 vert_eye, vec3 vert_dir, vec3 aabbmin, vec3 aabbmax,
                          out Ray r, out float s1, out float rtnear, out float rtfar);
//////////////////////////////////////////////////////////////////////////////////////////////////
// From common/utils.frag
vec4 GetFromTransferFunction (vec3 tex_pos);
float IntegrateRaySegment (out vec3 L, float alpha, float d, float T, vec3 Le, vec3 Li);
//////////////////////////////////////////////////////////////////////////////////////////////////

const float corr_fact = ApplyOpacityCorrectionFactor * OpacityCorrectionFactor; 
float EvaluationVoxelConeTracing (vec3 tex_pos)
{
  float Tvd = 1.0;

  // Get current world position
  vec3 realpos = tex_pos - (VolumeScaledSizes * 0.5);

  // Get point light vector
  vec3 cone_vec = normalize(WorldLightingPos - realpos);

  float apex_distance = ConeInitialStep;
  float step_size = ConeStepSize;
  float DXbase = 1.0;

  int is = 0;
  while(is < ConeNumberOfSamples)
  {
    float xl_x = (apex_distance + step_size * 0.5);

    // Equation 5, computing the current mipmap level for Quadrilinear interpolation
    float mm_level = log2((2.0 * xl_x * TanRadiusConeApexAngle) / DXbase);

    vec3 wpos = (tex_pos + cone_vec * xl_x);
    //if (wpos.x < 0 || wpos.y < 0 || wpos.z < 0 || wpos.x > VolumeScaledSizes.x
    //   || wpos.y > VolumeScaledSizes.y || wpos.z > VolumeScaledSizes.z) break;

    // Get current mean and standard deviation of the current sample from lod texture (clipmap for distributed rendering)
    vec2 g_ms = textureLod(TexSuperVoxelsVolume, (tex_pos + cone_vec * xl_x) / VolumeScaledSizes, mm_level).rg;
    float opacity = texture(TexPreIntegrationLookup, (g_ms.rg + vec2(0.5)) / vec2(VolumeMaxDensity, VolumeMaxStandardDeviation)).r;

    opacity = 1.0 - pow(1.0 - opacity, step_size * corr_fact);
    
    // Update current opacity
    Tvd *= (1.0 - opacity);
    
    apex_distance = apex_distance + step_size
      // * (mm_level + 1)
    ;

    step_size = step_size * ConeStepIncreaseRate;

    is = is + 1;
  }

  return Tvd;
}

void main (void)
{
  float T = 1.0;
  float s1 = 1.0;

  Ray r; float tnear, tfar;
  bool inbox = RayAABBIntersection(vert_eye, vert_dir, - VolumeScaledSizes * 0.5,
                                                       + VolumeScaledSizes * 0.5, r, s1, tnear, tfar);

  if (!inbox) discard;
  FragColor = vec4(1.0, 1.0, 1.0, 0.0);

  // Initialization Parameters
  vec3 L = vec3(0);
  float s = 0.0;

  // Get initial pos
  vec3 pos = r.Origin + r.Dir * tnear;
  // Translate pos [VolumeSizes/2, VolumeSizes/2] to pos_from_zero [0, VolumeSizes]
  pos = pos + (VolumeScaledSizes * 0.5);
  
  while (s < s1)
  {
    float d = min(StepSize, s1 - s);
    
    // Update Sample Position
    pos = pos + r.Dir * d;
    vec4 src = GetFromTransferFunction(pos);
    vec3 Li = src.rgb;

    // if alpha is > 0.0, apply shading
    if (src.a > 0.0)
    {
      float ka = 0.0, kd = 0.0, ks = 0.0;
      if (ApplyOcclusion == 1)
        ka = Kambient;

      float Ivd = 0.0;
      if (ApplyShadow == 1)
      {
        kd = Kdiffuse;
        ks = Kspecular;
        Ivd = EvaluationVoxelConeTracing(pos);
      }
      
      // Shading, combining "Directional Ambient Occlusion" and "Directional Shadows"
      if (ApplyPhongShading == 1)
      {
        vec3 Wpos = pos - (VolumeScaledSizes * 0.5);
        
        vec3 gradient_normal = texture(TexVolumeGradient, pos / VolumeScaledSizes).xyz;
        
        if (gradient_normal != vec3(0, 0, 0))
        {
          gradient_normal      = normalize(gradient_normal);
          
          vec3 light_direction = normalize(WorldLightingPos - Wpos);
          vec3 eye_direction   = normalize(r.Origin - Wpos);
          vec3 halfway_vector  = normalize(eye_direction + light_direction);
        
          float dot_diff = max(0, dot(gradient_normal, light_direction));
          float dot_spec = max(0, dot(halfway_vector, gradient_normal));

          Li = (1.0 / (ka + kd)) * (Li * ka + Ivd * (Li * kd * dot_diff)) 
            + Ivd * (ks * Ispecular * pow(dot_spec, Nshininess))
          ;
        }
      }
      else
      {
        Li = (1.0 / (ka + kd)) * (Li * ka + Li * Ivd * kd);
      }
    }
    	  
    // From "Local and Global Illumination in the Volume Rendering Integral" (2010)
    float expt = exp(-src.a * d);
    L += T * (1.0 - expt) * Li;
    T = T * expt;

    if (T < 0.01) break; 

    s = s + d;
  }
  
  FragColor = vec4(L, 1.0 - T);
}