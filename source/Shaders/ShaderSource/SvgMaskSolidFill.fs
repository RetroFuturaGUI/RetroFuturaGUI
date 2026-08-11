#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform vec4 uColor;
uniform sampler2D uMask;

void main()
{
    fragColor = vec4(uColor.rgb, uColor.a * texture(uMask, vTexCoord).r);
}
