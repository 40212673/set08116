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

map<string, HMesh> meshes_gate, meshes_normal, meshes_phong, meshes_blend, meshes_glowing, meshes_shadow;
effect eff_basic, eff_phong, eff_blend, eff_normal, eff_shadow, eff_shadow_main, eff_glowing;
map<string, texture> texs;
map<string, texture*> tex_maps;
free_camera free_cam;
arc_ball_camera arc_cam;
directional_light light, light_lava;
point_light light_glowing;
spot_light spot;
shadow_map shadow;
bool setFree = true;
double cursor_x = 0.0;
double cursor_y = 0.0;
float t = 0.0f;

bool initialise() {
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	return true;
}

// Function to create meshes
void createMeshes()
{
	// Create shadow map- use screen size
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());


	//Create Shadow objects
	meshes_shadow["plane"] = HMesh(geometry_builder::create_plane());
	meshes_shadow["plane"].texture_scale = 0.1;
	meshes_shadow["demon_teapot"] = HMesh(geometry("objects/teapot.obj"));
	meshes_shadow["demon_teapot"].get_transform().translate(vec3(-30.0, 0.0, 0.0));
	meshes_shadow["demon_teapot"].get_transform().scale = vec3(0.1, 0.1, 0.1);


	// Create and transform meshes_gate a.k.a the gate
	meshes_gate["column1"] = HMesh(geometry_builder::create_cylinder(1, 16, vec3(4.0f, 15.0f, 4.0f)));
	meshes_gate["column1"].texture_scale = 0.05;
	meshes_gate["column2"] = meshes_gate["column1"];
	meshes_gate["column2"].parent = &meshes_gate["column1"];
	meshes_gate["gate_ceiling"] = HMesh(geometry_builder::create_box(vec3(26.0f, 2.5f, 6.0f)));
	meshes_gate["gate_ceiling"].parent = &meshes_gate["column2"];
	meshes_gate["horn1"] = HMesh(geometry_builder::create_pyramid(vec3(6.0f, 8.0f, 6.0f)));
	meshes_gate["horn1"].texture_scale = 0.05;
	meshes_gate["horn1"].parent = &meshes_gate["gate_ceiling"];
	meshes_gate["horn2"] = meshes_gate["horn1"];
	meshes_gate["horn2"].parent = &meshes_gate["horn1"];


	// Build gate
	meshes_gate["column1"].get_transform().translate(vec3(-10.0f, 7.5f, 30.0f));
	meshes_gate["column2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	meshes_gate["gate_ceiling"].get_transform().translate(vec3(-10.0f, 7.5f, 0.0f));
	meshes_gate["horn1"].get_transform().translate(vec3(-10.0f, 5.0f, 0.f));
	meshes_gate["horn2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));


	// Set up Pentagrams for Phong

	meshes_phong["pentagram1"] = HMesh(geometry_builder::create_cylinder(1, 64, vec3(10.0f, 0.05f, 10.0f)));
	meshes_phong["pentagram1"].texture_scale = 0.1;
	meshes_phong["pool"] = HMesh(geometry("objects/pool.obj"));	
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
	meshes_glowing["sun"].get_transform().translate(vec3(140, 40, -20));
	meshes_blend["blend_planet"].get_transform().translate(vec3(0, 100, -100));
	meshes_glowing["demon_cube"] = HMesh(geometry_builder::create_box(vec3(5.0f, 5.0f, 5.0f)));
	meshes_glowing["demon_cube"].get_transform().translate(vec3(-10.0f, 20.0f, 0.0f));
	meshes_glowing["demon_cube"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>() / 2.0));
}

//Function to set up the materials
void setUpMaterials()
{
	material mat;
	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(500.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));

	meshes_phong["pentagram1"].set_material(mat);
	meshes_phong["pentagram2"].set_material(mat);

	mat.set_shininess(100.0f);
	mat.set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	for (auto &e : meshes_gate)
	{
		e.second.set_material(mat);
	}

	mat.set_emissive(vec4(0.4f, 0.0f, 0.0f, 1.0f));
	mat.set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);

	meshes_normal["lava"].set_material(mat);

	mat.set_emissive(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 0.0f, 1.0f));

	meshes_glowing["sun"].set_material(mat);

	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes_shadow["plane"].set_material(mat);
	mat.set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	meshes_shadow["demon_teapot"].set_material(mat);
}

// Function to set light properties
void setLight()
{
	// Set light properties for lava
	light_lava.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
	light_lava.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light_lava.set_direction(normalize(vec3(1.0f, 1.0f, 0.0f)));

	// Set light properties for sun
	light_glowing.set_position(vec3(meshes_glowing["sun"].get_transform().position.x, meshes_glowing["sun"].get_transform().position.y, meshes_glowing["sun"].get_transform().position.z));
	light_glowing.set_light_colour(vec4(0.8f, 0.0f, 0.0f, 0.0f));
	light_glowing.set_range(20.0f);

	// Set light properties for demon baby
	spot.set_position(vec3(-10.0f, 20.0f, 0.0f));
	spot.set_light_colour(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	spot.set_direction(normalize(vec3(-1, -1, 0)));
	spot.set_range(500.0f);
	spot.set_power(10.0f);

	// Update the shadow map light_position from the spot light
	shadow.light_position = spot.get_position();
	// do the same for light_dir property
	shadow.light_dir = spot.get_direction();

}

// Function to load textures
void loadTextures()
{
	// texs contains all the used texture
	texs["check"] = texture("textures/check_1.png");
	texs["gate"] = texture("textures/gate_red.png");
	texs["pentagram"] = texture("textures/pentagram.png");
	texs["blood"] = texture("textures/blood.png");
	texs["black_rock"] = texture("textures/black_rock.png");
	texs["blend_map"] = texture("textures/blend_map1.png");
	texs["lava"] = texture("textures/lava.png");
	texs["lava_normalmap"] = texture("textures/lava_normalmap.png");
	texs["sun"] = texture("textures/sun.png");
	texs["demon_baby"] = texture("textures/demon_baby.png");
	// tex_maps points to texturesin texs to reuse textures in some instances
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
	tex_maps["demon_cube"] = &(texs["demon_baby"]);
}

// Function to load shaders and build effects
void loadBuildEffects()
{
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
	eff_glowing.add_shader("shaders/point.frag", GL_FRAGMENT_SHADER);
	eff_glowing.add_shader("shaders/point.vert", GL_VERTEX_SHADER);
	eff_shadow_main.add_shader("shaders/shadow.vert", GL_VERTEX_SHADER);
	vector<string> frag_shaders{ "shaders/shadow.frag", "shaders/part_spot.frag", "shaders/part_shadow.frag" };
	eff_shadow_main.add_shader(frag_shaders, GL_FRAGMENT_SHADER);

	eff_shadow.add_shader("shaders/spot.vert", GL_VERTEX_SHADER);
	eff_shadow.add_shader("shaders/spot.frag", GL_FRAGMENT_SHADER);


	// Build effect  
	eff_basic.build();
	eff_phong.build();
	eff_blend.build();
	eff_normal.build();
	eff_glowing.build();
	eff_shadow_main.build();
	eff_shadow.build();
}

// Function to set up cameras
void setCameras()
{
	// Set free_camera properties  
	free_cam.set_position(vec3(0.0f, 10.0f, 0.0f));
	free_cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	free_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	// Set arc_cam properties
	arc_cam.set_target(meshes_phong["pentagram1"].get_transform().position);
	arc_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	arc_cam.set_distance(20);
}


bool load_content() {

	// Create the meshes
	createMeshes();

	//Set up materials
	setUpMaterials();

	// Set up Light
	setLight();

	// Load textures
	loadTextures();

	// Load shader and build effects
	loadBuildEffects();
  
	// Set up cameras
	setCameras();

	return true;
}


bool update(float delta_time) {

	// Set preferred camera
	if (glfwGetKey(renderer::get_window(), '1')) {
		setFree = true;
	}
	if (glfwGetKey(renderer::get_window(), '2')) {
		setFree = false;
	}

	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());
	// Get current x any position for mouse variable
	double current_x;
	double current_y;

	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	float delta_x = current_x - cursor_x;
	float delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;

	// if setFree then set up free camera
	if (setFree)
	{
		// Rotate free_cameras by delta
		// delta_y - x-axis rotation
		// delta_x - y-axis rotation
		free_cam.rotate(delta_x, -delta_y);
		// Use keyboard to move the free_camera - WSAD
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
		// Move free_camera
		free_cam.move(movement);

		// Update the free_camera
		free_cam.update(delta_time);
	}

	// otherwise set up arc ball camera
	else
	{
		// Rotate camera
		arc_cam.rotate(5 * delta_y, 5 * -delta_x);
		// The target object
		static HMesh &target_mesh = meshes_phong["pentagram1"];
		// The ratio of pixels to rotation - remember the fov
		static const float sh = static_cast<float>(renderer::get_screen_height());
		static const float sw = static_cast<float>(renderer::get_screen_height());
		static const double ratio_width = quarter_pi<float>() / sw;
		static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;

		// Use UP and DOWN to change camera distance
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
			arc_cam.set_distance(arc_cam.get_distance() + 6.0f * delta_time);
		}
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
			arc_cam.set_distance(arc_cam.get_distance() - 6.0f * delta_time);
		}
		// Update the camera
		arc_cam.update(delta_time);
	}

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	// Rotate pentagrams
	meshes_phong["pentagram1"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);
	meshes_phong["pentagram2"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()) * delta_time);

	// set linearly increasing t value for sine function
	t += delta_time;
	// Increase velocity
	meshes_phong["pentagram1"].get_transform().translate(vec3(0.0f, sin(t)*0.04 , 0.0f));
	meshes_phong["pentagram2"].get_transform().translate(vec3(0.0f, sin(t)*0.04, 0.0f));

	// Move lava in pool
	meshes_normal["lava"].texture_offset += vec2(0.07 * delta_time, 0.05 * delta_time);   

	return true;
}

// Method for Hierarchy creation
mat4 hierarchyCreation (HMesh *m)
{
	// Grab current M
	auto M = m->get_transform().get_transform_matrix();


	// Grab parents position until null
	if (m->parent != nullptr)
	{
		M = hierarchyCreation(m->parent) * M;
	}
	
	return M;
}
// Pentagrams and pool
void renderPhong(HMesh &m, string name)
{
	// Bind effect
	renderer::bind(eff_phong);
	// Create MVP matrix
	mat4 M;
	mat4 V;
	mat4 P;
	mat4 MVP;
	if (setFree)
	{
		M = m.get_transform().get_transform_matrix();
		V = free_cam.get_view();
		P = free_cam.get_projection();
		MVP = P * V * M;
	}
	else
	{
		M = m.get_transform().get_transform_matrix();
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		MVP = P * V * M;
	}
	// Set texture offset and scale
	glUniform1f(eff_phong.get_uniform_location("tex_scale"), m.texture_scale);
	glUniform2fv(eff_phong.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));

	// Set MVP matrix uniform
	glUniformMatrix4fv(eff_phong.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	glUniformMatrix4fv(eff_phong.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));

	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(eff_phong.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(light, "light");
	// Bind texture
	if (tex_maps.count(name)) {
		renderer::bind(*tex_maps[name], 0);
	}
	else {
		renderer::bind(texs["check"], 0);
	}
	// Set tex uniform
	glUniform1i(eff_phong.get_uniform_location("tex"), 0);
	// Set eye position - Get this from active free_camera
	glUniform3fv(eff_phong.get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
	// Render mesh
	renderer::render(m);
}
// Build Gate
void renderPhongGate(HMesh &m, string name)
{
	// Bind effect
	renderer::bind(eff_phong);
	// Create MVP Matrix
	mat4 V;
	mat4 P;
	// Set for free or arc ball camera
	if (setFree)
	{
		V = free_cam.get_view();
		P = free_cam.get_projection();
	}
	else
	{
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
	}
	// Hierarchy chain for the gate
	mat4 M = hierarchyCreation(&m);

	auto MVP = P * V * M;

	// Set texture offset and scale
	glUniform1f(eff_phong.get_uniform_location("tex_scale"), m.texture_scale);
	glUniform2fv(eff_phong.get_uniform_location("texture_offset"), 1, value_ptr(m.texture_offset));

	// Set MVP matrix uniform
	glUniformMatrix4fv(eff_phong.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	glUniformMatrix4fv(eff_phong.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(eff_phong.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(light, "light");
	// Bind texture
	if (tex_maps.count(name)) {
		renderer::bind(*tex_maps[name], 0);
	}
	else {
		renderer::bind(texs["check"], 0);
	}
	// Set tex uniform
	glUniform1i(eff_phong.get_uniform_location("tex"), 0);
	// Set eye position - Get this from active free_camera
	glUniform3fv(eff_phong.get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
	// Render mesh
	renderer::render(m);
}
// Blend Planet
void renderBlend(HMesh &m)
{

	// Bind effect
	renderer::bind(eff_blend);
	// Create MVP matrix dependant on camera
	mat4 M;
	mat4 V;
	mat4 P;
	mat4 MVP;
	if (setFree)
	{
		M = m.get_transform().get_transform_matrix();
		V = free_cam.get_view();
		P = free_cam.get_projection();
		MVP = P * V * M;
	}
	else
	{
		M = m.get_transform().get_transform_matrix();
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		MVP = P * V * M;
	}
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
// Lava pool
void renderNormal(HMesh &m)
{

	// Bind effect
	renderer::bind(eff_normal);
	// Create MVP matrix dependant on camera
	mat4 M;
	mat4 V;
	mat4 P;
	mat4 MVP;
	if (setFree)
	{
		M = m.get_transform().get_transform_matrix();
		V = free_cam.get_view();
		P = free_cam.get_projection();
		MVP = P * V * M;
	}
	else
	{
		M = m.get_transform().get_transform_matrix();
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		MVP = P * V * M;
	}
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
	glUniform3fv(eff_normal.get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
	// Render the mesh
	renderer::render(m);
}
// Demon baby and sun
void renderGlowing(HMesh &m, string name)
{
	// Bind effect
	renderer::bind(eff_glowing);
	// Create MVP matrix dependant on camera
	mat4 M;
	mat4 V;
	mat4 P;
	mat4 MVP;
	if (setFree)
	{
		M = m.get_transform().get_transform_matrix();
		V = free_cam.get_view();
		P = free_cam.get_projection();
		MVP = P * V * M;
	}
	else
	{
		M = m.get_transform().get_transform_matrix();
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		MVP = P * V * M;
	}
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff_glowing.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff_glowing.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff_glowing.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));

	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(light_glowing, "point");
	// Bind texture
	if (tex_maps.count(name)) {
		renderer::bind(*tex_maps[name], 0);
	}
	else {
		renderer::bind(texs["check"], 0);
	}
	// Set tex uniform
	glUniform1i(eff_glowing.get_uniform_location("tex"), 0);
	// Set eye position
	glUniform3fv(eff_glowing.get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
	// Render the mesh
	renderer::render(m);
}
// Plane and teacup
void renderShadow(map<string, HMesh> hmeshes)
{
	// Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);

	mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);

	renderer::bind(eff_shadow);
	
	for (auto &e : hmeshes) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_shadow.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
													// Render mesh
		renderer::render(m);
	}

	// Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);
	// Bind shader
	renderer::bind(eff_shadow_main);


	for (auto &e : hmeshes) {
		auto m = e.second;
		// Create MVP matrix dependant on camera
		mat4 M;
		mat4 V;
		mat4 P;
		mat4 MVP;
		if (setFree)
		{
			M = m.get_transform().get_transform_matrix();
			V = free_cam.get_view();
			P = free_cam.get_projection();
			MVP = P * V * M;
		}
		else
		{
			M = m.get_transform().get_transform_matrix();
			V = arc_cam.get_view();
			P = arc_cam.get_projection();
			MVP = P * V * M;
		}
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_shadow_main.get_uniform_location("MVP"), // Location of uniform
			1,                                    // Number of values - 1 mat4
			GL_FALSE,                             // Transpose the matrix?
			value_ptr(MVP));                      // Pointer to matrix data
												  // Set M matrix uniform
		glUniformMatrix4fv(eff_shadow_main.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(eff_shadow_main.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(m.get_transform().get_normal_matrix()));
		// Set lightMVP uniform, using:
		//Model matrix from m
		auto lM = m.get_transform().get_transform_matrix();
		// viewmatrix from the shadow map
		auto lV = shadow.get_view();
		// Multiply together with LightProjectionMat
		auto lightMVP = LightProjectionMat * lV * lM;
		// Set uniform
		glUniformMatrix4fv(eff_shadow_main.get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		// Bind spot light
		renderer::bind(spot, "spot");
		// Bind texture
		renderer::bind(texs["black_rock"], 0);
		// Set tex uniform 
		glUniform1i(eff_shadow_main.get_uniform_location("tex"), 0);
		// Set eye position
		glUniform3fv(eff_shadow_main.get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 1);
		// Set the shadow_map uniform
		glUniform1i(eff_shadow_main.get_uniform_location("shadow_map"), 1);
		// Render mesh
		renderer::render(m);
	}
}

bool render() {

	// Render meshes_phong pentagrams
	for (auto &e : meshes_phong) {
		auto m = e.second;
		renderPhong(m, e.first);
	}
	
	// Render meshes_gate a.k.a gate
	for (auto &e : meshes_gate) {
		auto m = e.second;
		renderPhongGate(m, e.first);
	}
	
	// Render planet blend map a.k.a the planet
	renderBlend(meshes_blend["blend_planet"]);

	// Render normal mapped lava
	renderNormal(meshes_normal["lava"]);

	// Render glowing Sun and demon baby cube
	for (auto &e : meshes_glowing) {
		// Set light position for sun or demon baby
		if (e.first == "sun")
		{
			light_glowing.set_position(vec3(meshes_glowing["sun"].get_transform().position.x,
				meshes_glowing["sun"].get_transform().position.y,
				meshes_glowing["sun"].get_transform().position.z));
		}
		else
		{
			light_glowing.set_position(vec3(meshes_glowing["demon_cube"].get_transform().position.x,
				meshes_glowing["demon_cube"].get_transform().position.y,
				meshes_glowing["demon_cube"].get_transform().position.z));
		}
		renderGlowing(e.second, e.first);
	}

	// Render teapot and plane shadows
	renderShadow(meshes_shadow);

	
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