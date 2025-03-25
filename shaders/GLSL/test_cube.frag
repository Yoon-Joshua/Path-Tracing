#version 460

layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 OutColor;

layout(set = 1, binding = 0) uniform Camera{
    mat4 view;
    mat4 proj;
}camera;

// layout(set = 1, binding = 1) uniform Light{
//     vec3 position;
//     uint type;
//     vec3 direction;
//     // float padding0;
//     vec3 intensity;
//     // float padding1;
// }light;

void main(){
    OutColor = vec4(worldNormal * 0.5 + 0.5, 1);
}

