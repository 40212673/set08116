#include <glm\glm.hpp>
#include <graphics_framework.h>
#include <memory>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh m;
mesh m2;
effect eff;
target_camera cam;
texture tex, tex2, tex3, tex4, tex5;
vec3 pos(0.0f, 0.0f, 0.0f);
float s = 1.0f;

bool load_content() {
  // Construct geometry object
  geometry geom;
  geometry geom2;
  // Create triangle data
  // Positions
  vector<vec3> positions{vec3(1.0f, 1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f), vec3(-1.0f, -1.0f, 0.0f),
	  vec3(1.0f, 1.0f, 0.0f), vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f) };

  vector<vec3> topPositions{ vec3(1.0f, 1.0f, -2.0f), vec3(-1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f),
	  vec3(1.0f, 1.0f, -2.0f), vec3(-1.0f, 1.0f, -2.0f), vec3(-1.0f, 1.0f, 0.0f) };
  // *********************************
  // Define texture coordinates for triangle
  vector<vec2> tex_coords{ vec2(1.0f,1.0f), vec2(0.0f, 1.0f), vec2( 0.0f, 0.0f), vec2(1.0f, 1.0f),
								vec2(0.0f, 0.0f), vec2(1.0f, 0.0f)};
  // *********************************
  // Add to the geometry
  geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
  geom2.add_buffer(topPositions, BUFFER_INDEXES::POSITION_BUFFER);
  // *********************************
  // Add texture coordinate buffer to geometry
  geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
  geom2.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
  // *********************************

  // Create mesh object
  m = mesh(geom);
  m2 = mesh(geom2);

  // Load in texture shaders here
  eff.add_shader("27_Texturing_Shader/simple_texture.vert", GL_VERTEX_SHADER);
  eff.add_shader("27_Texturing_Shader/simple_texture.frag", GL_FRAGMENT_SHADER);
  // *********************************
  // Build effect
  eff.build();
  // Load texture "textures/sign.jpg"
  tex = texture("textures/sign.jpg");
  tex2 = texture("textures/checker.png");
  tex3 = texture("textures/blend_map1.png");
  tex4 = texture("textures/sahara_ft.jpg");
  tex5 = texture("textures/check_2.png");
  // *********************************


  // Set camera properties
  cam.set_position(vec3(10.0f, 10.0f, 10.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);

  return true;
}

vec2 rotate1;

bool update(float delta_time) {
	static double x;
	static double y;
	double newX, newY;
	glfwGetCursorPos(renderer::get_window(), &newX, &newY);
	rotate1 += vec2(newX - x, newY - y);
	x = newX;
	y = newY;
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
		pos += vec3(0.0f, 0.0f, -5.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
		pos += vec3(0.0f, 0.0f, 5.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
		pos += vec3(-5.0f, 0.0f, 0.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
		pos += vec3(5.0f, 0.0f, 0.0f) * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_O)) {
		s /= 1.05f;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_P)) {
		s *= 1.05f;
	}
  // Update the camera
  cam.update(delta_time);
  return true;
}

bool render() {
  // Bind effect
  renderer::bind(eff);
  mat4 T, R, S;
  T = translate(mat4(1.0f), pos);
  S = scale(mat4(1.0f), vec3(s, s, s));
  R = rotate(mat4(1.0f), rotate1.x*0.01f, vec3(0.0f, 1.0f, 0.0f));
  // Create MVP matrix
  auto M = m.get_transform().get_transform_matrix() * T*R*S;
  auto Mat2 = m2.get_transform().get_transform_matrix() * T*R*S;
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto MVP = P * V * M;
  auto Mat2VP = P * V * Mat2;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
                     1,                               // Number of values - 1 mat4
                     GL_FALSE,                        // Transpose the matrix?
                     value_ptr(MVP)); 
  

  // Pointer to matrix data
  glUniformMatrix4fv(eff.get_uniform_location("M2VP"), // Location of uniform
	  1,                               // Number of values - 1 mat4
	  GL_FALSE,                        // Transpose the matrix?
	  value_ptr(Mat2VP));

  // *********************************
  // Bind texture to renderer
  renderer::bind(tex, 0);
  renderer::bind(tex2, 1);
  renderer::bind(tex3, 2);
  renderer::bind(tex4, 3);
  renderer::bind(tex5, 5);
  // Set the texture value for the shader here
  glUniform1i(eff.get_uniform_location("tex"), 0);
  renderer::render(m);

  glUniform1i(eff.get_uniform_location("tex"), 5);
  renderer::render(m2);


  mat4 M2 = translate(M, vec3(0.0f, 0.0f, -2.0f)) * rotate(mat4(1.0f), pi<float>() , vec3(0.0f, 1.0f, 0.0f));
  MVP = P * V * M2;

  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
	  1,                               // Number of values - 1 mat4
	  GL_FALSE,                        // Transpose the matrix?
	  value_ptr(MVP));
  // Render the mesh
  glUniform1i(eff.get_uniform_location("tex"), 1);
  renderer::render(m);


  mat4 M3 = translate(M, vec3(1.0f, 0.0f, -1.0f)) * rotate(mat4(1.0f), pi<float>()/2, vec3(0.0f, 1.0f, 0.0f));
  MVP = P * V * M3;

  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
	  1,                               // Number of values - 1 mat4
	  GL_FALSE,                        // Transpose the matrix?
	  value_ptr(MVP));
  glUniform1i(eff.get_uniform_location("tex"), 2);
  renderer::render(m);

  //

  mat4 M4 = translate(M, vec3(-1.0f, 0.0f, -1.0f)) * rotate(mat4(1.0f), -pi<float>() / 2, vec3(0.0f, 1.0f, 0.0f));
  MVP = P * V * M4;

  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
	  1,                               // Number of values - 1 mat4
	  GL_FALSE,                        // Transpose the matrix?
	  value_ptr(MVP));
  glUniform1i(eff.get_uniform_location("tex"), 3);
  renderer::render(m);

  return true;
}

void main() {
  // Create application
  app application("27_Texturing_Shader");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}