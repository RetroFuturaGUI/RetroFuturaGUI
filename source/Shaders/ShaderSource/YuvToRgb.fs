#version 330 core

in vec2 PictureCoord;
out vec4 FragColor;

uniform sampler2D uY;
uniform sampler2D uU;
uniform sampler2D uV;
uniform vec3 uOffset;    // black level and chroma centre, as the texture stores them (0..1)
uniform vec3 uScale;     // stretches the codes to Y 0..1 and Cb/Cr -0.5..0.5
uniform mat3 uYuvToRgb;  // the colour matrix: BT.601, BT.709 or BT.2020

void main()
{
	vec3 yuv = vec3(texture(uY, PictureCoord).r, texture(uU, PictureCoord).r, texture(uV, PictureCoord).r);
	vec3 rgb = uYuvToRgb * ((yuv - uOffset) * uScale);
	FragColor = vec4(clamp(rgb, 0.0, 1.0), 1.0);
}
