#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes_basic, meshes_normal, meshes_light, meshes_blend;
std::array<map<string, mesh>*, 5> meshes_basic_hierarchy;
effect eff_basic, lighting_eff, normal_eff;
map<string, texture> texs;
map<string, texture*> tex_maps;
free_camera cam;
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
  
	// TODO set hierarchy
	// Create and transform meshes_basic a.k.a the gate
	meshes_basic["column1"] = mesh(geometry_builder::create_cylinder());
	meshes_basic["column1"].get_transform().scale = vec3(4.0f, 15.0f, 4.0f);
	meshes_basic["column2"] = meshes_basic["column1"];
	meshes_basic["gate_ceiling"] = mesh(geometry_builder::create_box(vec3(26.0f, 2.5f, 6.0f)));
	meshes_basic["horn1"] = mesh(geometry_builder::create_pyramid(vec3(6.0f, 8.0f, 6.0f)));
	meshes_basic["horn2"] = meshes_basic["horn1"];
	
  
	// Build gate

	meshes_basic["column1"].get_transform().translate(vec3(-10.0f, 7.5f, 30.0f));
	meshes_basic["column2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	//meshes_basic["gate_ceiling"].get_transform().translate(vec3(0.0f, 15.0f, 30.0f));
	meshes_basic["gate_ceiling"].get_transform().translate(vec3(-10.0f, 7.5f, 0.0f));
	//meshes_basic["horn1"].get_transform().translate(vec3(-10.0f, 20.0f, 30.f));
	meshes_basic["horn1"].get_transform().translate(vec3(-10.0f, 5.0f, 0.f));
	meshes_basic["horn2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
  
	// Other meshes

	meshes_normal["pentagram1"] = mesh(geometry_builder::create_cylinder(1, 64, vec3(10.0f, 0.05f, 10.0f)));
	meshes_normal["plane"] = mesh(geometry_builder::create_plane());
	meshes_normal["pool"] = mesh(geometry("objects/pool.obj"));

	// Set up Pentagrams
	meshes_normal["pentagram1"].get_transform().translate(vec3(27.5f, 8.0f, 30.0f));
	meshes_normal["pentagram1"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes_normal["pentagram2"] = meshes_normal["pentagram1"];
	meshes_normal["pentagram2"].get_transform().translate(vec3(-55.0f, 0.0f, 0.0f));

	// Set up pool
	meshes_normal["pool"].get_transform().scale = vec3(20.0f);
	meshes_normal["pool"].get_transform().translate(vec3(0.0f, -0.01f, -30.0f));

	// Load gate into array for Hierarchy chain
	int i = 0;
	for (auto &e : meshes_basic)
	{
		meshes_basic_hierarchy[0] = e;
		i++;
	}

	// Load texture  
	texs["check"] = texture("textures/check_1.png");
	tex_maps["column1"] = &(texs["check"]);
	tex_maps["column2"] = &(texs["check"]);

	// Load in shaders  
	eff_basic.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);  
	eff_basic.add_shader("shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
  
	// Build effect  
	eff_basic.build();

  
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
	meshes_normal["pentagram1"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);
	meshes_normal["pentagram2"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);

	t += delta_time;
	// Move pentagrams up and down
	if (velocity_pent == 1.0 || velocity_pent == -1.0)
		incrementor_pent = -incrementor_pent;
	// Increase velocity
	meshes_normal["pentagram1"].get_transform().translate(vec3(0.0f, sin(t)*0.04 , 0.0f));
	meshes_normal["pentagram2"].get_transform().translate(vec3(0.0f, sin(t)*0.04, 0.0f));

	// Update the camera
	cam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	velocity_pent += incrementor_pent;

	return true;
}

bool render() {
	// Render meshes_basic
	for (auto &e : meshes_basic) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff_basic);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;

		// Hierarchy chain for the gate
		for (size_t j = meshes_basic.; j > 0; j--)
		{
			M = meshes_basic[j - 1].get_transform().get_transform_matrix() * M;
		}

		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_basic.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind and set texture 
		if (tex_maps.count(e.first)){
			renderer::bind(*tex_maps[e.first], 0);
		}
		else {
			renderer::bind(texs["check"], 0);
		}
		glUniform1i(eff_basic.get_uniform_location("tex"), 0);
		// Render mesh
		renderer::render(m);
	}
	
	// Render meshes_normal
	for (auto &e : meshes_normal) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff_basic);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;

		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_basic.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind and set texture 
		if (tex_maps.count(e.first)) {
			renderer::bind(*tex_maps[e.first], 0);
		}
		else {
			renderer::bind(texs["check"], 0);
		}
		glUniform1i(eff_basic.get_uniform_location("tex"), 0);
		// Render mesh
		renderer::render(m);
	}
	
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