#version 330 core
in vec2 TexCoords;
out vec4 color;
uniform sampler2D colorTex;
uniform vec3 selCol;
uniform bool isSelected;
uniform bool isHovered;
vec3 Invert(vec3 src){ return vec3(1 - src.r, 1 - src.g, 1 - src.b); }
bool InBounds(vec2 src, float bnd) {
    return src.x < bnd || src.x > 1-bnd || src.y < bnd || src.y > 1-bnd;}
void main() {    
    float modif = isSelected ? 0.4 : 1;
    vec3 chngCol = selCol;
    if(isHovered && !isSelected){
        modif = 0.85;
        chngCol = vec3(1.0, 1.0, 1.0);}
    vec4 tex = texture(colorTex, TexCoords);
    vec3 rgbPart = tex.rgb * modif + chngCol * (1 - modif);
    float alpha = tex.a * modif + (1 - modif);
    float bnd = isSelected ? 0.05 : 0;
    if(InBounds(TexCoords,bnd))
        rgbPart = Invert(rgbPart);
    color = vec4(rgbPart, alpha);
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