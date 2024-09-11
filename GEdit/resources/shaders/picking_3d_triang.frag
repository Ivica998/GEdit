#version 330 core

out vec4 color;

uniform vec4 PickingColor;
//uniform int multirend;

void main(){
    
    float index = (gl_PrimitiveID + 1) / 255.0;
    color = vec4(PickingColor.xy ,index ,1.0f);

}