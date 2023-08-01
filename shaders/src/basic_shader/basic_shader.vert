#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 uvCoord;

layout(location = 0) out vec2 fragUvCoord;

layout(binding = 0) uniform RectLocation{
    mat4 location;
};

void main() {
    gl_Position = location * vec4(inPosition, 1.0);
    fragUvCoord = uvCoord;
}