#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform int RGBA;


void main()
{

    vec4 textureColor = texture(ourTexture, TexCoord);
    int sRGBA = RGBA;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    if(sRGBA == 1)
    {
        r = textureColor.a;
        g = textureColor.a;
        b = textureColor.a;
        a = 1.0f;
        FragColor = vec4(r,g,b,a);
        return;
    }

    if( sRGBA > 8)
    {
        r = textureColor.r;
        sRGBA = sRGBA - 8;
    }
    if( sRGBA > 4)
    {
        g = textureColor.g;
        sRGBA = sRGBA - 4;
    }

    if( sRGBA > 2)
    {
        b = textureColor.b;
        sRGBA = sRGBA - 2;
    }

    if(sRGBA > 1)
    {
        a = textureColor.a;
    }


    FragColor = vec4(r,g,b,a);
}