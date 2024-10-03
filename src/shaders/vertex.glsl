#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform vec2 OFFSET;
uniform float AspectRatio;
uniform mat4x4 ProjMat;

out vec2 TexCoord;

void main()
{
    gl_Position = ProjMat * vec4(aPos.x, aPos.y, aPos.z, 1.0) + vec4(OFFSET,0,0);
    TexCoord = vec2(aTexCoord.x,-aTexCoord.y);
}