#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh m1;
mesh m2;
effect eff;
target_camera cam;

bool load_content() {
  // Construct geometry object
  geometry geom1;
  geometry geom2;
  // Create triangle data
  // Positions
  vector<vec3> positions1{vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f)};
  vector<vec3> positions2{ vec3(2.0f, 1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(3.0f, -1.0f, 0.0f) };
  // Colours
  vector<vec4> colours{vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)};
  // Add to the geometry
  geom1.add_buffer(positions1, BUFFER_INDEXES::POSITION_BUFFER);
  geom1.add_buffer(colours, BUFFER_INDEXES::COLOUR_BUFFER);
  geom2.add_buffer(positions2, BUFFER_INDEXES::POSITION_BUFFER);
  geom2.add_buffer(colours, BUFFER_INDEXES::COLOUR_BUFFER);
  // *********************************
  // Create mesh object here
  m1 = mesh(geom1);
  m2 = mesh(geom2);
  // *********************************

  // Load in shaders
  eff.add_shader("shaders/basic.vert", GL_VERTEX_SHADER);
  eff.add_shader("shaders/basic.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_position(vec3(10.0f, 10.0f, 10.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  // Use keys to update transform values
  // WSAD - movement
  // Cursor - rotation
  // O decrease scale, P increase scale
  // Use the mesh functions, I've left two of the IFs as a hint
  if (glfwGetKey(renderer::get_window(), 'W')) {
    m1.get_transform().position -= vec3(0.0f, 0.0f, 5.0f) * delta_time;
  }
  // *********************************
  if (glfwGetKey(renderer::get_window(), 'S')) {
	  m1.get_transform().position -= vec3(0.0f, 0.0f, -5.0f) * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), 'A')) {
	  m1.get_transform().position -= vec3(5.0f, 0.0f, 0.0f) * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), 'D')) {
	  m1.get_transform().position -= vec3(-5.0f, 0.0f, 0.0f) * delta_time;
  }
  // *********************************
  //if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
  //  m.get_transform().rotate(vec3(-pi<float>() * delta_time, 0.0f, 0.0f));
  //}
  // *********************************
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
	  m2.get_transform().position -= vec3(0.0f, 0.0f, 5.0f) * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
	  m2.get_transform().position -= vec3(0.0f, 0.0f, -5.0f) * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) {
	  m2.get_transform().position -= vec3(-5.0f, 0.0f, 0.0f) * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) {
	  m2.get_transform().position -= vec3( 5.0f, 0.0f, 0.0f) * delta_time;
  }
  // *********************************
  if (glfwGetKey(renderer::get_window(), 'O')) {
	  m1.get_transform().scale *= 1.1;
  }
  if (glfwGetKey(renderer::get_window(), 'P')) {
	  m1.get_transform().scale /= 1.1;
  }
  // Update the camera
  cam.update(delta_time);
  return true;
}

bool render() {
  // Bind effect
  renderer::bind(eff);
  mat4 M1;
  mat4 M2;
  // *********************************
  // Get the model transform from the mesh
  M1 = m1.get_transform().get_transform_matrix();
  // *********************************
  // Create MVP matrix
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto MVP = P * V * M1;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // *********************************
  // Render the mesh here
  renderer::render(m1);
  // *********************************

  M2 = m2.get_transform().get_transform_matrix();
  // *********************************
  // Create MVP matrix
  MVP = P * V * M2;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // *********************************
  // Render the mesh here
  renderer::render(m2);
  return true;
}

void main() {
  // Create application
  app application("25_Transforming_Meshes");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}