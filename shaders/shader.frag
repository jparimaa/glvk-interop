#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUv;

layout(binding = 1) uniform sampler2D sharedImage;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(inColor.r, inColor.g, inColor.b, 1.0) * 0.2 + texture(sharedImage, inUv) * 0.8;
}
