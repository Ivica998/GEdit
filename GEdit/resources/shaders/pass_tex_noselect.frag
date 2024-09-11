#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D colorTex;

void main()
{    
    vec4 tex = texture(colorTex, TexCoords);
    color = tex;
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