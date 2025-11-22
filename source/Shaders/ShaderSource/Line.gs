#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float uLineThickness;
uniform vec4 uLineLengths;
uniform vec2 uViewportSize;

void main() {
    vec2 direction = gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy;

    vec2 normalized = normalize(vec2(-direction.y, direction.x)); // normalize direction
    normalized *= (uLineThickness / uViewportSize.y); // scale thiccness by pixels

    gl_Position = vec4(gl_in[0].gl_Position.xy + normalized, 0.0, 1.0); // top left
    EmitVertex();

    gl_Position = vec4(gl_in[0].gl_Position.xy - normalized, 0.0,  uLineLengths[1]); // bottom left
    EmitVertex();

    gl_Position = vec4(gl_in[1].gl_Position.xy + normalized, 0.0, 1.0); // top right
    EmitVertex();

    gl_Position = vec4(gl_in[1].gl_Position.xy - normalized, 0.0, 1.0); // bottom right
    EmitVertex();

    EndPrimitive();
}
