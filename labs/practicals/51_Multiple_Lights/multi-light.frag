#version 440

// Point light information
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

// Spot light data
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};

// Material data
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Point lights being used in the scene
uniform point_light points[4];
// Spot lights being used in the scene
uniform spot_light spots[5];
// Material of the object being rendered
uniform material mat;
// Position of the eye (camera)
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;

// Incoming position
layout(location = 0) in vec3 vertex_position;
// Incoming normal
layout(location = 1) in vec3 transformed_normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord_out;

// Outgoing colour
layout(location = 0) out vec4 colour;

// Point light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                     in vec4 tex_colour) {
  // *********************************
 // Get distance between point light and vertex
  float d = distance(point.position, vertex_position);
  // Calculate attenuation factor
  float f = (point.constant + (point.linear * d) + (point.quadratic * d * d));
  // Calculate light colour
  vec4 light_colour = point.light_colour/f;
  light_colour.a = 1.0f;
  // Calculate light dir
  vec3 light_dir = normalize(point.position - vertex_position);
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient

  // Calculate k
  float k1 = max(dot(transformed_normal, light_dir), 0.0f);
  // Calculate diffuse
    vec4 colour1 = vec4(1,1,1,1);
  vec4 diffuse = k1 * (mat.diffuse_reflection * light_colour);
  // Calculate view direction
  view_dir = normalize(eye_pos - vertex_position);
  // Calculate half vector
  vec3 half_vector  = normalize(light_dir + view_dir);
  // Calculate specular component
  // Calculate k
  float k2 = pow(max(dot(transformed_normal, half_vector), 0.0f), mat.shininess);
  // Calculate specular
  vec4 specular = k2 * (mat.specular_reflection * light_colour);
  // Sample texture
  vec4 texture_sample = texture(tex, tex_coord_out);
  // Calculate primary colour component
  vec4 primary = (mat.emissive + diffuse);
  // Calculate final colour - remember alpha
  primary.a = 1.0f;
  colour1 = (primary * texture_sample) + specular;
  colour1.a = 1.0f;
  // *********************************
  return colour1;
}

// Spot light calculation
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                    in vec4 tex_colour) {
// *********************************
  // Calculate direction to the light
  vec4 colour1 = vec4(1,1,1,1);
  vec3 light_dir = normalize(spot.position - vertex_position);
  // Calculate distance to light
  float d = distance(spot.position, vertex_position);
  // Calculate attenuation value
  float f = (spot.constant + (spot.linear * d) + (spot.quadratic * d * d));
  // Calculate spot light intensity
  float j = pow(max(dot(-spot.direction, light_dir), 0.0f), spot.power);
  // Calculate light colour
  vec4 light_colour = spot.light_colour*(j/f);
  light_colour.a = 1.0f;
  // Calculate view direction

  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient

    // Calculate k
  float k1 = max(dot(transformed_normal, light_dir), 0.0f);
  // Calculate diffuse
  vec4 diffuse = k1 * (mat.diffuse_reflection * light_colour);
  // Calculate view direction
  view_dir = normalize(eye_pos - vertex_position);
  // Calculate half vector
  vec3 half_vector  = normalize(light_dir + view_dir);
  // Calculate specular component
  // Calculate k
  float k2 = pow(max(dot(transformed_normal, half_vector), 0.0f), mat.shininess);
  // Calculate specular
  vec4 specular = k2 * (mat.specular_reflection * light_colour);
  // Sample texture
  vec4 texture_sample = texture(tex, tex_coord_out);
  // Calculate primary colour component
  vec4 primary = (mat.emissive + diffuse);
  // Calculate final colour - remember alpha
  primary.a = 1.0f;
  colour1 = (primary * texture_sample) + specular;
  colour1.a = 1.0f;
  // *********************************
  return colour1;
}

void main() {

  colour = vec4(0.0, 0.0, 0.0, 1.0);
  // *********************************
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - vertex_position);
  // Sample texture
  vec4 texture_sample = texture(tex, tex_coord_out);
  // Sum point lights
  for (int i = 0; i < 4; ++i) {
	colour += calculate_point(points[i], mat, vertex_position, transformed_normal, view_dir, texture_sample);
  }
  // Sum spot lights
    for (int i = 0; i < 5; ++i) {
	colour += calculate_spot(spots[i], mat, vertex_position, transformed_normal, view_dir, texture_sample);
  }
  // *********************************
}