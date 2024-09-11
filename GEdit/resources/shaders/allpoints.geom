#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

//float aspect in;
in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fColor;

void main() {   
    fColor = gs_in[0].color;
    float val = 0.1;
    for(int i = 0; i < 4; i++)
    {
        gl_Position = gl_in[0].gl_Position + vec4(-0.1, -0.1, 0.0, 0.0); 
        EmitVertex();   
    }

    gl_Position = gl_in[0].gl_Position + vec4( 0.1, -0.1, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( -0.1, 0.1, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.1, 0.0, 0.0);
    EmitVertex();
    
    EndPrimitive();
} 