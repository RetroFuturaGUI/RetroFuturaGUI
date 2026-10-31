#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram

in vec2 vTexCoord;
out vec4 fragColor;

uniform vec4 uColor;
uniform sampler2D uMask;

void main()
{
    fragColor = vec4(uColor.rgb, uColor.a * texture(uMask, vTexCoord).r);
    fragColor.a *= uOpacity; //faded with the widget and whatever fades around it
}
