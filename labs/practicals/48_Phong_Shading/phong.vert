#version 440

// The model matrix
uniform mat4 M;
// The transformation matrix
uniform mat4 MVP;
// The normal matrix
uniform mat3 N;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming texture coordinates
layout(location = 10) in vec2 tex_coord_in;

// Outgoing position
// Outgoing normal
layout(location = 4) out vec3 n_t;
// Outgoing texture coordinate
layout(location = 2) out vec2 coord_tex_out;
layout(location = 0) out vec3 pos_vec;

void main() {
  // Set position
  gl_Position = MVP * vec4(position, 1);
  // *********************************
  // Output other values to fragment shader;
  pos_vec = vec3(M * vec4(position, 1.0));
  n_t = N * normal;
  coord_tex_out = tex_coord_in;
  // *********************************
}