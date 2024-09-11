#version 330 core
layout (location = 0) out vec4 FragColor;
  
uniform vec4 color;
uniform bool selected;

void main()
{
    vec4 result = selected ? vec4(1.0,1.0,0.0,0.5) : color;
    FragColor = result;
} 