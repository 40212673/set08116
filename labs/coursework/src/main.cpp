#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
geometry geom;
effect eff;
texture tex;
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
  
	// Create and transform meshes
	meshes["plane"] = mesh(geometry_builder::create_plane());
	meshes["column1"] = mesh(geometry_builder::create_cylinder());
	meshes["gate_ceiling"] = mesh(geometry_builder::create_box(vec3(26.0f, 2.5f, 6.0f)));
	meshes["horn1"] = mesh(geometry_builder::create_pyramid(vec3(6.0f, 8.0f, 6.0f)));
	meshes["pentagram1"] = mesh(geometry_builder::create_cylinder(1, 64, vec3(10.0f, 0.05f, 10.0f)));
  
	// Build gate
	meshes["column1"].get_transform().scale = vec3(4.0f, 15.0f, 4.0f);
	meshes["column1"].get_transform().translate(vec3(-10.0f, 7.5f, 30.0f));
	meshes["column2"] = meshes["column1"];
	meshes["column2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	meshes["gate_ceiling"].get_transform().translate(vec3(0.0f, 15.0f, 30.0f));
	meshes["horn1"].get_transform().translate(vec3(-10.0f, 20.0f, 30.f));
	meshes["horn2"] = meshes["horn1"];
	meshes["horn2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
  
	// Set up Pentagrams
	meshes["pentagram1"].get_transform().translate(vec3(27.5f, 8.0f, 30.0f));
	meshes["pentagram1"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes["pentagram2"] = meshes["pentagram1"];
	meshes["pentagram2"].get_transform().translate(vec3(-55.0f, 0.0f, 0.0f));

	// Load texture  
	tex = texture("textures/check_1.png");
  
  
	// Load in shaders  
	eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);  
	eff.add_shader("shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
  
	// Build effect  
	eff.build();

  
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
	meshes["pentagram1"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);
	meshes["pentagram2"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);

	t += delta_time;
	// Move pentagrams up and down
	if (velocity_pent == 1.0 || velocity_pent == -1.0)
		incrementor_pent = -incrementor_pent;
	// Increase velocity
	meshes["pentagram1"].get_transform().translate(vec3(0.0f, sin(t)*0.04 , 0.0f));
	meshes["pentagram2"].get_transform().translate(vec3(0.0f, sin(t)*0.04, 0.0f));

	// Update the camera
	cam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	velocity_pent += incrementor_pent;

	return true;
}

bool render() {
	// Render meshes
	for (auto &e : meshes) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind and set texture
		renderer::bind(tex, 0);
		glUniform1i(eff.get_uniform_location("tex"), 0);
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