#version 330 core
in vec2 TexCoords;
layout(location = 0) out float color;

uniform sampler2D colorTex;

void main()
{    
    vec2 flipped = TexCoords;//   vec2(TexCoords.x,1-TexCoords.y);
    vec4 tex = texture(colorTex, flipped);
    color = tex.r;
}










//#version 330 core
//layout (location = 0) out vec4 FragColor;
//
//uniform vec3 lightColor;
//
//void main()
//{           
//    FragColor = vec4(lightColor, 1.0);
//}