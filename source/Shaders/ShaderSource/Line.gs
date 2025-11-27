#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;
uniform mat4 uProjection;
uniform float uLineThickness;
in vec4 vs_WorldPosition[];

void main()
{
    vec4 p0_world = vs_WorldPosition[0];
    vec4 p1_world = vs_WorldPosition[1];
    vec2 direction = p1_world.xy - p0_world.xy;
    float length = length(direction);

    if (length > 0.0)
    {
        vec2 normal = (vec2(-direction.y, direction.x) / length) * uLineThickness;
        vec4 p0 = uProjection * vec4(p0_world.xy + normal, p0_world.z, 1.0);
        vec4 p1 = uProjection * vec4(p0_world.xy - normal, p0_world.z, 1.0);
        vec4 p2 = uProjection * vec4(p1_world.xy + normal, p1_world.z, 1.0);
        vec4 p3 = uProjection * vec4(p1_world.xy - normal, p1_world.z, 1.0);

        gl_Position = p0;
        EmitVertex();
        gl_Position = p1;
        EmitVertex();
        gl_Position = p2;
        EmitVertex();
        gl_Position = p3;
        EmitVertex();
    }
    else 
    {
        // Fallback (emit a default line)
        vec4 p0 = uProjection * vec4(p0_world.xy, p0_world.z, 1.0);
        vec4 p1 = uProjection * vec4(p1_world.xy, p1_world.z, 1.0);
        gl_Position = p0;
        EmitVertex();
        gl_Position = p1;
        EmitVertex();
        EndPrimitive();
    }
    EndPrimitive();
}