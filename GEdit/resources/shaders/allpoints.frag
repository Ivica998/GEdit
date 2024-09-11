#version 330 core
layout (location = 0) out vec4 FragColor;

uniform vec4 color;

float make_circle(vec2 cord, float radius){
    float distance = radius - length(cord);
    distance = step(0.0,distance);
    return distance;
}

in vec3 fColor;

void main()
{
/*
    vec3 color = vec3(1,0,0);
    float radius = 0.01;

    vec2 cord = gl_FragCoord.xy;
    float rez = make_circle(cord,radius);
    vec3 almost = vec3(rez) * color;
    FragColor = vec4(almost,1.0);
*/
    FragColor = vec4(fColor,1.0);
}