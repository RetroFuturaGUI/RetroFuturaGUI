// Cuts what is drawn to the ClipScopes around it (Shader::ClipScope): a vertex is kept on the side of each plane its normal
// points to. Only the planes a ClipScope uses are switched on; the rest are ignored, whatever they hold.
uniform vec4 uClipPlanes[8];

out float gl_ClipDistance[8];

// Measures the vertex against every plane, at its place in the window before the projection.
void clipToScopes(vec4 placedPosition)
{
    for(int plane = 0; plane < 8; ++plane)
        gl_ClipDistance[plane] = dot(uClipPlanes[plane], placedPosition);
}
