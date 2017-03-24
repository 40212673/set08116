#version 430 core

// Incoming texture containing frame information
uniform sampler2D tex;

// Our colour filter - calculates colour intensity
const vec3 intensity = vec3(0.299, 0.587, 0.184);

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample texture colour
  vec4 tex_colour = texture(tex, tex_coord);
  // Calculate grey value
  float grey_colour = dot(vec3(tex_colour), intensity);
  // Use greyscale to as final colour
  // - ensure alpha is 1
  colour = vec4(grey_colour, grey_colour, grey_colour, 1.0);
  // *********************************
}