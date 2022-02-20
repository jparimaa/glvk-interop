#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;

layout(binding = 0) uniform UBO
{
    vec4 color;
}
ubo;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUv;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    outColor = ubo.color;
    outUv = inUv;
}
