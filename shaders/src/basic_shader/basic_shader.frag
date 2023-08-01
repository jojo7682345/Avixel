#version 450

layout(location = 0) in vec2 uvCoord;

layout(location = 0) out vec4 outColor;

#define STYLE_HAS_TEXTURE 1

layout(binding = 1) uniform Style{
    uint styleFlags;
    vec4 color;
};
layout(binding = 2) uniform sampler2D image;

vec4 getTextureSample(vec2 coord){
    if((styleFlags & STYLE_HAS_TEXTURE) != 0){
        return texture(image, coord);
    }
    return vec4(1.0,1.0,1.0,1.0);
}

void main() {
    outColor = getTextureSample(uvCoord) * color;
}