#version 430

layout(location = 0) in vec3 VerPos;

out vec3 world_pos;
out vec2 CVector;
out vec4 pos_clip_space;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform vec3 CameraDirection;
uniform float DistanceFromNearSlice;

// https://learnopengl.com/Getting-Started/Coordinate-Systems
// https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
// http://watkins.cs.queensu.ca/~jstewart/454/notes/454-MVPtransform/
// https://www.khronos.org/opengl/wiki/Vertex_Transformation
// https://www.ntu.edu.sg/home/ehchua/programming/opengl/CG_BasicsTheory.html
void main (void)
{
  vec3 vpos = VerPos + CameraDirection * DistanceFromNearSlice;

	gl_Position = ProjectionMatrix * ViewMatrix * vec4(vpos, 1.0);
  pos_clip_space = gl_Position;

  //////////////////////////////
  // Equation 12
  vec4 viewpos = ViewMatrix * vec4(vpos, 1.0);
  vec4 v_scale = ProjectionMatrix * vec4(1, 1, viewpos.z, 1);
  CVector = vec2(v_scale.x/v_scale.w, v_scale.y/v_scale.w);

  world_pos = vpos;
}