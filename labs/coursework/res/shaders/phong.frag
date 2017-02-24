#version 440

// A directional light structure
struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};

// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Directional light for the scene
uniform directional_light light;
// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;

//Adjusts for texture
uniform float tex_scale;
uniform vec2 texture_offset;

// Incoming position
// Incoming normal
layout(location = 4) in vec3 trans_n;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;
layout(location = 0) in vec3 vec_position;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {

  // *********************************
  // Calculate ambient component
  vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
  // Calculate k
  float k1 = max(dot(trans_n, light.light_dir), 0.0f);
  // Calculate diffuse
  vec4 diffuse = k1 * (mat.diffuse_reflection * light.light_colour);
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - vec_position);
  // Calculate half vector
  vec3 half_vector  = normalize(light.light_dir + view_dir);
  // Calculate specular component
  // Calculate k
  float k2 = pow(max(dot(trans_n, half_vector), 0.0f), mat.shininess);
  // Calculate specular
  vec4 specular = k2 * (mat.specular_reflection * light.light_colour);
  // Sample texture
  vec4 texture_sample = texture(tex, (tex_coord * tex_scale) + texture_offset);
  // Calculate primary colour component
  vec4 primary = (mat.emissive + ambient + diffuse);
  // Calculate final colour - remember alpha
  primary.a = 1.0f;
  colour = (primary * texture_sample + specular);
  // *********************************
}