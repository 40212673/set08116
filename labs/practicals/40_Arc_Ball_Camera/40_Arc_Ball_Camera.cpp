#include <glm\glm.hpp>
#include <glm\gtx\component_wise.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect eff;
texture tex;
arc_ball_camera cam;
double cursor_x = 0.0;
double cursor_y = 0.0;

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
  // Create plane mesh
  meshes["plane"] = mesh(geometry_builder::create_plane());

  // Create scene
  meshes["box"] = mesh(geometry_builder::create_box());
  meshes["tetra"] = mesh(geometry_builder::create_tetrahedron());
  meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
  meshes["disk"] = mesh(geometry_builder::create_disk(20));
  meshes["cylinder"] = mesh(geometry_builder::create_cylinder(20, 20));
  meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
  meshes["torus"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));

  // Transform objects
  meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
  meshes["tetra"].get_transform().scale = vec3(4.0f, 4.0f, 4.0f);
  meshes["tetra"].get_transform().translate(vec3(-30.0f, 10.0f, -10.0f));
  meshes["pyramid"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["pyramid"].get_transform().translate(vec3(-10.0f, 7.5f, -30.0f));
  meshes["disk"].get_transform().scale = vec3(3.0f, 1.0f, 3.0f);
  meshes["disk"].get_transform().translate(vec3(-10.0f, 11.5f, -30.0f));
  meshes["disk"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  meshes["cylinder"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["cylinder"].get_transform().translate(vec3(-25.0f, 2.5f, -25.0f));
  meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
  meshes["sphere"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));

  // Create mesh to chase
  meshes["chaser"] = mesh(geometry_builder::create_box());
  meshes["chaser"].get_transform().position = vec3(0.0f, 0.5f, 0.0f);

  // Load texture
  tex = texture("textures/checker.png");

  // Load in shaders
  eff.add_shader("27_Texturing_Shader/simple_texture.vert", GL_VERTEX_SHADER);
  eff.add_shader("27_Texturing_Shader/simple_texture.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_target(meshes["chaser"].get_transform().position);
  cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  // The target object
  static mesh &target_mesh = meshes["chaser"];

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
  // Multiply deltas by ratios and delta_time - gets actual change in orientation
  delta_x *= ratio_width * delta_time;
  delta_y *= ratio_height * delta_time;// Rotate cameras by delta
  // delta_y - x-axis rotation
  // delta_x - y-axis rotation
  cam.rotate(5 * delta_y, 5 * -delta_x);
  // Use keyboard to move the target_mesh- WSAD
  // Also remember to translate camera
  if (glfwGetKey(renderer::get_window(), 'W')) {
	  meshes["chaser"].get_transform().translate(vec3(0.0f, 0.0f, -5.0f) * delta_time);
	  cam.translate(vec3(0.0f, 0.0f, -5.0f) * delta_time);
  }
  if (glfwGetKey(renderer::get_window(), 'S')) {
	  meshes["chaser"].get_transform().translate(vec3(0.0f, 0.0f, 5.0f) * delta_time);
	  cam.translate(vec3(0.0f, 0.0f, 5.0f) * delta_time);
  }
  if (glfwGetKey(renderer::get_window(), 'A')) {
	  meshes["chaser"].get_transform().translate(vec3(-5.0f, 0.0f, 0.0f) * delta_time);
	  cam.translate(vec3(-5.0f, 0.0f, 0.0f) * delta_time);
  }
  if (glfwGetKey(renderer::get_window(), 'D')) {
	  meshes["chaser"].get_transform().translate(vec3(5.0f, 0.0f, 0.0f) * delta_time);
	  cam.translate(vec3(5.0f, 0.0f, 0.0f) * delta_time);
  }
  // Use UP and DOWN to change camera distance
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
	  cam.set_distance(cam.get_distance() + 6.0f * delta_time);
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
	  cam.set_distance(cam.get_distance() - 6.0f * delta_time);
  }
  // Update the camera
  cam.update(delta_time);
  // Update cursor pos
  cursor_x = current_x;
  cursor_y = current_y;
  // *********************************
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
  app application("40_Arc_Ball_Camera");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}