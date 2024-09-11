#version 330 core
in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;

uniform sampler2D colorTex;
uniform vec4 textColor;
uniform vec4 bgColor;
uniform float width;
void main()
{    
    vec2 flipped = vec2(TexCoords.x,1-TexCoords.y);
    //flipped = TexCoords;
    float alpha = texture(colorTex, flipped).r;
    //if(alpha > 0) alpha = 1;
    vec4 result = textColor * vec4(1.0, 1.0, 1.0, alpha);

    float totalAlpha = alpha;
    vec4 borderColor = {0,0,0,1};
    vec4 color = textColor;
    if(false){
        float offset = 0/width;
        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right    
        );
        float sur = 1;
        float mid = 1;
        float kernel[9] = float[](
            sur, sur, sur,
            sur, mid, sur,
            sur, sur, sur
        );    
        float sampleAlpha[9];
        for(int i = 0; i < 9; i++) {
            sampleAlpha[i] = texture(colorTex, flipped + offsets[i]).r;
        }      
        for(int i = 0; i < 9; i++){
            if(sampleAlpha[i] == 0)
                color = borderColor;
            totalAlpha += sampleAlpha[i] * kernel[i];
        }
    }
    /*
    if(totalAlpha > 0)
        totalAlpha = 1;
    */
    //vec4 result2 = color * totalAlpha + bgColor * (1 - totalAlpha);
    vec4 result2 = color * vec4(1.0, 1.0, 1.0, totalAlpha);
    if(totalAlpha == 0)
        discard;
    //FragColor = vec4(result2.xyz, 1.0);
    FragColor = result2;
}  