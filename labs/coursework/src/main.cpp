#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Class created for ease of Hierarchy chain and setting texture offset and scale
class HMesh : public mesh
{
public:
	HMesh* parent;
	float texture_scale;
	vec2 texture_offset;

	HMesh(geometry g) : mesh (g)
	{	
		parent = nullptr;
		texture_scale = 1.0f;
		texture_offset = vec2(0.0f, 0.0f);
	}

	HMesh() : mesh()
	{

	}
};

map<string, HMesh> meshes_basic, meshes_normal, meshes_phong, meshes_blend, meshes_glowing, meshes_shadow;
effect eff_basic, eff_phong, eff_blend, eff_normal, eff_shadow, eff_glowing;
map<string, texture> texs;
map<string, texture*> tex_maps;
free_camera cam;
directional_light light, light_lava;
spot_light light_glowing;
double cursor_x = 0.0;
double cursor_y = 0.0;
float incrementor_pent = 0.2f;
float velocity_pent = 0.0f;
float t = 0.0f;

bool initialise() {
	// *********************************
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	// *********************************
	return true;
}

bool load_content() {
	
	//Create Blend plane
	meshes_phong["plane"] = HMesh(geometry_builder::create_plane());
	meshes_phong["plane"].texture_scale = 0.1;

	// Create and transform meshes_basic a.k.a the gate
	meshes_basic["column1"] = HMesh(geometry_builder::create_cylinder(1, 16, vec3(4.0f, 15.0f, 4.0f)));
	meshes_basic["column1"].texture_scale = 0.05;
	meshes_basic["column2"] = meshes_basic["column1"];
	meshes_basic["column2"].parent = &meshes_basic["column1"];
	meshes_basic["gate_ceiling"] = HMesh(geometry_builder::create_box(vec3(26.0f, 2.5f, 6.0f)));
	meshes_basic["gate_ceiling"].parent = &meshes_basic["column2"];
	meshes_basic["horn1"] = HMesh(geometry_builder::create_pyramid(vec3(6.0f, 8.0f, 6.0f)));
	meshes_basic["horn1"].texture_scale = 0.05;
	meshes_basic["horn1"].parent = &meshes_basic["gate_ceiling"];
	meshes_basic["horn2"] = meshes_basic["horn1"];
	meshes_basic["horn2"].parent = &meshes_basic["horn1"];


	// Build gate
	meshes_basic["column1"].get_transform().translate(vec3(-10.0f, 7.5f, 30.0f));
	meshes_basic["column2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	meshes_basic["gate_ceiling"].get_transform().translate(vec3(-10.0f, 7.5f, 0.0f));
	meshes_basic["horn1"].get_transform().translate(vec3(-10.0f, 5.0f, 0.f));
	meshes_basic["horn2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	
  
	// Other meshes

	meshes_phong["pentagram1"] = HMesh(geometry_builder::create_cylinder(1, 64, vec3(10.0f, 0.05f, 10.0f)));
	meshes_phong["pentagram1"].texture_scale = 0.1;
	meshes_phong["pool"] = HMesh(geometry("objects/pool.obj"));

	// Set up Pentagrams
	meshes_phong["pentagram1"].get_transform().translate(vec3(27.5f, 8.0f, 30.0f));
	meshes_phong["pentagram1"].texture_offset = vec2(2.446f, 2.446f);
	meshes_phong["pentagram1"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes_phong["pentagram2"] = meshes_phong["pentagram1"];
	meshes_phong["pentagram2"].get_transform().translate(vec3(-55.0f, 0.0f, 0.0f));

	// Set up pool
	meshes_phong["pool"].get_transform().scale = vec3(20.0f);
	meshes_phong["pool"].get_transform().translate(vec3(0.0f, -0.01f, -30.0f));
	meshes_normal["lava"] = HMesh(geometry_builder::create_plane(29, 25));
	meshes_normal["lava"].get_transform().translate(vec3(0, 3.4, -29)); 

	// Set up sun and demonic baby cube and blend planet
	meshes_glowing["sun"] = HMesh(geometry_builder::create_sphere(32, 32));
	meshes_glowing["sun"].get_transform().scale = vec3(8, 8, 8);
	meshes_glowing["sun"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes_blend["blend_planet"] = meshes_glowing["sun"];
	//meshes_glowing["sun"].get_transform().translate(vec3(80, 80, -40));
	meshes_blend["blend_planet"].get_transform().translate(vec3(0, 100, -100));

	//Set up materials
	material mat;
	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(500.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));

	meshes_phong["pentagram1"].set_material(mat);
	meshes_phong["pentagram2"].set_material(mat);

	mat.set_shininess(100.0f);
	mat.set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	for (auto &e : meshes_basic)
	{ 
		e.second.set_material(mat);
	}

	mat.set_emissive(vec4(0.4f, 0.0f, 0.0f, 1.0f));
	mat.set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	
	meshes_normal["lava"].set_material(mat);

	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 0.0f, 1.0f));

	meshes_glowing["sun"].set_material(mat);

	// Set light properties for lava
	light_lava.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
	light_lava.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light_lava.set_direction(normalize(vec3(1.0f, 1.0f, 0.0f)));

	// Set light properties for sun
	light_glowing.set_position(vec3(meshes_glowing["sun"].get_transform().position.x, meshes_glowing["sun"].get_transform().position.y, meshes_glowing["sun"].get_transform().position.z));
	light_glowing.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light_glowing.set_direction(normalize(vec3(0.0f, -1.0f, 0.0f)));
	light_glowing.set_range(20.0f);
	light_glowing.set_power(5.0f);

	// Load texture  
	texs["check"] = texture("textures/check_1.png");
	texs["gate"] = texture("textures/gate_red.png");
	texs["pentagram"] = texture("textures/pentagram.png");
	texs["blood"] = texture("textures/blood.png");
	texs["black_rock"] = texture("textures/black_rock.png");
	texs["blend_map"] = texture("textures/blend_map1.png");
	texs["lava"] = texture("textures/lava.png");
	texs["lava_normalmap"] = texture("textures/lava_normalmap.png");
	texs["sun"] = texture("textures/sun.png"); 

	tex_maps["column1"] = &(texs["gate"]);
	tex_maps["column2"] = &(texs["gate"]);
	tex_maps["gate_ceiling"] = &(texs["gate"]);
	tex_maps["horn1"] = &(texs["gate"]);
	tex_maps["horn2"] = &(texs["gate"]);
	tex_maps["pentagram1"] = &(texs["pentagram"]);
	tex_maps["pentagram2"] = &(texs["pentagram"]);
	tex_maps["pool"] = &(texs["black_rock"]);
	tex_maps["lava"] = &(texs["lava"]);
	tex_maps["sun"] = &(texs["sun"]);


	// Load in shaders
	eff_basic.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);  
	eff_basic.add_shader("shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
	eff_phong.add_shader("shaders/phong.vert", GL_VERTEX_SHADER);
	eff_phong.add_shader("shaders/phong.frag", GL_FRAGMENT_SHADER);
	eff_blend.add_shader("shaders/blend.vert", GL_VERTEX_SHADER);
	eff_blend.add_shader("shaders/blend.frag", GL_FRAGMENT_SHADER);
	eff_normal.add_shader("shaders/normal.vert", GL_VERTEX_SHADER);
	eff_normal.add_shader("shaders/normal.frag", GL_FRAGMENT_SHADER);
	eff_normal.add_shader("shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	eff_normal.add_shader("shaders/part_normal_map.frag", GL_FRAGMENT_SHADER);
	eff_glowing.add_shader("shaders/spot.frag", GL_FRAGMENT_SHADER);
	eff_glowing.add_shader("shaders/spot.vert", GL_VERTEX_SHADER);

  
	// Build effect  
	eff_basic.build();
	eff_phong.build();
	eff_blend.build();
	eff_normal.build();
	eff_glowing.build();

  
	// Set camera properties  
	cam.set_position(vec3(0.0f, 10.0f, 0.0f));  
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));  
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);  
	return true;
}


bool update(float delta_time) {
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;
	// *********************************
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	float delta_x = current_x - cursor_x;
	float delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;
	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	cam.rotate(delta_x, -delta_y);
	// Use keyboard to move the camera - WSAD
	vec3 movement;
	if (glfwGetKey(renderer::get_window(), 'W')) {
		movement = vec3(0.0f, 0.0f, 20.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		movement = vec3(0.0f, 0.0f, -20.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		movement = vec3(-20.0f, 0.0f, 0.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		movement = vec3(20.0f, 0.0f, 0.0f) * delta_time;
	}
	// Move camera
	cam.move(movement);

	// Rotate pentagrams
	meshes_phong["pentagram1"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);
	meshes_phong["pentagram2"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);

	t += delta_time;
	// Move pentagrams up and down
	if (velocity_pent == 1.0 || velocity_pent == -1.0)
		incrementor_pent = -incrementor_pent;
	// Increase velocity
	meshes_phong["pentagram1"].get_transform().translate(vec3(0.0f, sin(t)*0.04 , 0.0f));
	meshes_phong["pentagram2"].get_transform().translate(vec3(0.0f, sin(t)*0.04, 0.0f));

	// Update the camera
	cam.update(delta_time);

	// Move lava in pool
	meshes_normal["lava"].texture_offset += vec2(0.07 * delta_time, 0.05 * delta_time);   

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	velocity_pent += incrementor_pent;

	return true;
}

mat4 hierarchyCreation (HMesh *m)
{
	// Create MVP matrix
	auto M = m->get_transform().get_transform_matrix();



	if (m->parent != nullptr)
	{
		M = hierarchyCreation(m->parent) * M;
	}
	
	return M;
}

// Function for blended plane

void renderBlend(HMesh &m)
{

	// Bind effect
	renderer::bind(eff_blend);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff_blend.get_uniform_location("MVP"), // Location of uniform
		1,                               // Number of values - 1 mat4
		GL_FALSE,                        // Transpose the matrix?
		value_ptr(MVP));                 // Pointer to matrix data

	// bind textures
	renderer::bind(texs["black_rock"], 0); 
	renderer::bind(texs["blood"], 1);
	renderer::bind(texs["blend_map"], 2);

	// Set the uniform values for textures
	static int tex_indices[] = { 0, 1 };
	glUniform1iv(eff_blend.get_uniform_location("tex"), 2, tex_indices);
	glUniform1i(eff_blend.get_uniform_location("blend"), 2);

	// Set texture offset and scale
	glUniform1f(eff_blend.get_uniform_location("tex_scale"), m.texture_scale);
	glUniform2fv(eff_blend.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));

	// Render the mesh
	renderer::render(m);
}

void renderNormal(HMesh &m)
{

	// Bind effect
	renderer::bind(eff_normal);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff_normal.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff_normal.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff_normal.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));

	// Set texture offset and scale
	glUniform1f(eff_normal.get_uniform_location("tex_scale"), m.texture_scale);
	glUniform2fv(eff_normal.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));

	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(light_lava, "light");
	// Bind texture
	renderer::bind(texs["lava"], 0);
	// Set tex uniform
	glUniform1i(eff_normal.get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(texs["lava_normalmap"], 1);
	// Set normal_map uniform
	glUniform1i(eff_normal.get_uniform_location("normal_map"), 1);
	// Set eye position
	glUniform3fv(eff_normal.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	// Render the mesh
	renderer::render(m);
}

void renderGlowing(HMesh &m)
{
	// Bind effect
	renderer::bind(eff_glowing);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff_glowing.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff_glowing.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff_glowing.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));

	// Set texture offset and scale
	//glUniform1f(eff_glowing.get_uniform_location("tex_scale"), m.texture_scale);
	//glUniform2fv(eff_glowing.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));

	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(light_glowing, "spot");
	// Bind texture
	renderer::bind(texs["sun"], 0);
	// Set tex uniform
	glUniform1i(eff_glowing.get_uniform_location("tex"), 0);
	// Set eye position
	glUniform3fv(eff_glowing.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	// Render the mesh
	renderer::render(m);
}

bool render() {

	// Render meshes_phong pentagrams

	for (auto &e : meshes_phong) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff_phong);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;

		// Set texture offset and scale
		glUniform1f(eff_phong.get_uniform_location("tex_scale"), m.texture_scale);
		glUniform2fv(eff_phong.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));
		 
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_phong.get_uniform_location("MVP"), // Location of uniform
			1,                               // Number of values - 1 mat4
			GL_FALSE,                        // Transpose the matrix?
			value_ptr(MVP));                 // Pointer to matrix data

											 // Set M matrix uniform

		glUniformMatrix4fv(eff_phong.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));   

		// ********************************* 
		// Set N matrix uniform - remember - 3x3 matrix
		glUniformMatrix3fv(eff_phong.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		// Bind light
		renderer::bind(light, "light");
		// Bind texture
		if (tex_maps.count(e.first)) {
			renderer::bind(*tex_maps[e.first], 0);
		}
		else {
		renderer::bind(texs["check"], 0);
		}
		// Set tex uniform
		glUniform1i(eff_phong.get_uniform_location("tex"), 0);
		// Set eye position - Get this from active camera
		glUniform3fv(eff_phong.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
		// Render mesh
		renderer::render(m);
		// *********************************
	}
	
	// Render meshes_basic
	for (auto &e : meshes_basic) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff_phong);

		auto V = cam.get_view();
		auto P = cam.get_projection();
		
		// Hierarchy chain for the gate
		mat4 M = hierarchyCreation(&m);

		auto MVP = P * V * M;

		// Set texture offset and scale
		glUniform1f(eff_phong.get_uniform_location("tex_scale"), m.texture_scale);
		glUniform2fv(eff_phong.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));

		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_phong.get_uniform_location("MVP"), // Location of uniform
			1,                               // Number of values - 1 mat4
			GL_FALSE,                        // Transpose the matrix?
			value_ptr(MVP));                 // Pointer to matrix data

											 // Set M matrix uniform

		glUniformMatrix4fv(eff_phong.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));

		// ********************************* 
		// Set N matrix uniform - remember - 3x3 matrix
		glUniformMatrix3fv(eff_phong.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		// Bind light
		renderer::bind(light, "light");
		// Bind texture
		if (tex_maps.count(e.first)) {
			renderer::bind(*tex_maps[e.first], 0);
		}
		else {
			renderer::bind(texs["check"], 0);
		}
		// Set tex uniform
		glUniform1i(eff_phong.get_uniform_location("tex"), 0);
		// Set eye position - Get this from active camera
		glUniform3fv(eff_phong.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
		// Render mesh
		renderer::render(m);
	}
	
	// Render planet blend map
	renderBlend(meshes_blend["blend_planet"]);

	// Render normal mapped lava
	renderNormal(meshes_normal["lava"]);

	// Render glowing Sun

	renderGlowing(meshes_glowing["sun"]);


	
	return true;
}

void main() {
  // Create application
  app application("Graphics Coursework");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}