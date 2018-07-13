#version 430 core

const float verts[24] = float[]
  (
    1, 0,
    0.866025, 0.5,
    0.5, 0.866025,
    0, 1,
    -0.5, 0.866025,
    -0.866025, 0.5,
    -1, 0,
    -0.866025, -0.5,
    -0.5, -0.866025,
    0, -1,
    0.5, -0.866025,
    0.866025, -0.5
  );

layout(location = 0) uniform vec3 offset_scale = vec3(0.0, 0.0, 1.0);

void main(){
  if(gl_VertexID == 0)
    gl_Position.xy = vec2(0, 0)*offset_scale.z + offset_scale.xy;
  else
    gl_Position.xy = vec2(verts[gl_VertexID%12*2], verts[gl_VertexID%12*2+1])*offset_scale.z + offset_scale.xy;
  gl_Position.z = 0.0;
  gl_Position.w = 1.0;
}
