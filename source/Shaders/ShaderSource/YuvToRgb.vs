#version 330 core

// One triangle that covers the whole render target - no vertex buffer, its corners come from gl_VertexID
out vec2 PictureCoord;

void main()
{
	vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2); // (0,0) (2,0) (0,2)
	gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);

	// the planes hold the picture top row first, the target is filled bottom row first like every
	// other texture - so sample upside down
	PictureCoord = vec2(corner.x, 1.0 - corner.y);
}
