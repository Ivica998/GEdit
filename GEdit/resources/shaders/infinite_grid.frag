#version 330 core
layout(location = 0) out vec4 outColor;

in vec2 TexCoords;

uniform sampler2D diffuseMap;

void main() {
    vec4 color = texture(diffuseMap, TexCoords);
    outColor = color;
}

