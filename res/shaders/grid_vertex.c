#version 430 core

const float verts[8] = float[]
  (
    -1.0, 0.0,
    1.0, 0.0,
    0.0, -1.0,
    0.0, 1.0
  );

layout(location = 0) uniform vec3 offset_scale = vec3(0.0, 0.0, 1.0);

void main(){

  gl_Position.xy = vec2(verts[gl_VertexID*2], verts[gl_VertexID*2+1])*offset_scale.z + offset_scale.xy;
  gl_Position.z = 0.0;
  gl_Position.w = 1.0;
}
