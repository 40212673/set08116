// Directional light structure
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT
struct directional_light
{
	vec4 ambient_intensity;
	vec4 light_colour;
	vec3 light_dir;
};
#endif

// A material structure
#ifndef MATERIAL
#define MATERIAL
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};
#endif

// Calculates the directional light
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir, in vec4 tex_colour)
{
 // *********************************
	// Calculate ambient component
<<<<<<< HEAD
	vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
	// Calculate diffuse component
	float k1 = max(dot(normal, light.light_dir), 0.0f);
    // Calculate diffuse
    vec4 diffuse = k1 * (mat.diffuse_reflection * light.light_colour);
	// Calculate half vector
	vec3 half_vector  = normalize(light.light_dir + view_dir);
	// Calculate specular component
	float k2 = pow(max(dot(normal , half_vector), 0.0f), mat.shininess);
    // Calculate specular
    vec4 specular = k2 * (mat.specular_reflection * light.light_colour);
=======

	// Calculate diffuse component :  (diffuse reflection * light_colour) *  max(dot(normal, light direction), 0)

	// Calculate normalized half vector 

	// Calculate specular component : (specular reflection * light_colour) * (max(dot(normal, half vector), 0))^mat.shininess

>>>>>>> 532fee74f852bc58232352f666163a3ba59925c5
 // *********************************
	// Calculate colour to return
	vec4 colour1 = ((mat.emissive + ambient + diffuse) * tex_colour) + specular;
	colour1.a = 1.0;
	// Return colour
	return colour1;

}
