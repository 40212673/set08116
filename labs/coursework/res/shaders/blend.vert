#version 440

uniform mat4 MVP;

// Set texture scale and offset uniform
uniform float tex_scale;
uniform vec2 texture_offset;

// *********************************
// Declare incoming values
// 0 - position
layout (location = 0) in vec3 position;
// 10 - tex_coord_in
layout (location = 10) in vec2 tex_coord_in;
// *********************************

// Outgoing value
layout (location = 0) out vec2 tex_coord_out;

void main()
{
	// Transform the position onto screen
	gl_Position = MVP * vec4(position, 1.0);
	// Output texture coordinates
	tex_coord_out = (tex_coord_in * tex_scale) + texture_offset;;
} 