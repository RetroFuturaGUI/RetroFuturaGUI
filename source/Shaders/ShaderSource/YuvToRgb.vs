#version 330 core

// One triangle that covers the whole render target - no vertex buffer, its corners come from gl_VertexID
out vec2 PictureCoord;
out float gl_ClipDistance[8];

void main()
{
	// Never cut: it fills a texture, even while a ClipScope around the Video's draw has clip planes switched on.
	for(int plane = 0; plane < 8; ++plane)
		gl_ClipDistance[plane] = 1.0;

	vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2); // (0,0) (2,0) (0,2)
	gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);

	// the planes hold the picture top row first, the target is filled bottom row first like every
	// other texture - so sample upside down
	PictureCoord = vec2(corner.x, 1.0 - corner.y);
}
