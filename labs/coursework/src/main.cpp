#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace std::chrono;
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

// Maximum number of particles
const unsigned int MAX_PARTICLES = 16000;

vec4 positions[MAX_PARTICLES];
vec4 velocitys[MAX_PARTICLES];

GLuint G_Position_buffer, G_Velocity_buffer;


map<string, HMesh> meshes_gate, meshes_normal, meshes_phong, meshes_blend, meshes_glowing, meshes_skybox, meshes_ground;
effect eff_basic, eff_phong, eff_blend, eff_normal, eff_glowing, sky_eff, ground_eff, eff_smoke, compute_eff, eff_godraysFirst, eff_godraysSecond;
map<string, texture> texs;
map<string, texture*> tex_maps;
free_camera free_cam;
arc_ball_camera arc_cam;
directional_light light, light_lava;
point_light light_glowing;
spot_light spot;
cubemap cube_map;
frame_buffer frameGodFirst;
frame_buffer frameGodTwo;
GLuint vao;
GLuint pvao;
geometry screen_quad;
bool setFree = true;
double cursor_x = 0.0;
double cursor_y = 0.0;
float t = 0.0f;
bool toggleGodray = false;
int status = 0;

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
	meshes_gate["column1"].get_transform().translate(vec3(-45.0f, 0.75f, 40.0f));
	meshes_gate["column1"].get_transform().rotate(vec3(0.0f, -half_pi<float>()/2.0, 0.0f));
	meshes_gate["column1"].get_transform().scale *= 0.1;
	meshes_gate["column2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	meshes_gate["gate_ceiling"].get_transform().translate(vec3(-10.0f, 7.5f, 0.0f));
	meshes_gate["horn1"].get_transform().translate(vec3(-10.0f, 5.0f, 0.f));
	meshes_gate["horn2"].get_transform().translate(vec3(20.0f, 0.0f, 0.0f));
	

	// Create the Skybox
	meshes_skybox["skybox"] = HMesh(geometry_builder::create_box());
	meshes_skybox["skybox"].get_transform().scale *= 1000;
	


	// Set up Pentagrams for Phong
	meshes_phong["pentagram1"] = HMesh(geometry_builder::create_cylinder(1, 64, vec3(10.0f, 0.05f, 10.0f)));
	meshes_phong["pentagram1"].texture_scale = 0.1;
	meshes_phong["pentagram1"].get_transform().translate(vec3(22.5f, 60.0f, 3.0f));
	meshes_phong["pentagram1"].texture_offset = vec2(2.446f, 2.446f);
	meshes_phong["pentagram1"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes_phong["pentagram2"] = meshes_phong["pentagram1"];
	meshes_phong["pentagram2"].get_transform().translate(vec3(-55.0f, 0.0f, 0.0f));

	// Set up pool
	meshes_normal["lava"] = HMesh(geometry_builder::create_cylinder(1, 64, vec3(30.0f, 0.05f, 30.0f)));
	meshes_normal["lava"].texture_scale = 0.5;
	meshes_normal["lava"].get_transform().translate(vec3(-3.8, 30.0, 2));

	// Set up sun and demonic baby cube and blend planet
	meshes_glowing["sun"] = HMesh(geometry_builder::create_sphere(32, 32));
	meshes_glowing["sun"].get_transform().scale = vec3(8, 8, 8);
	meshes_glowing["sun"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes_blend["blend_planet"] = meshes_glowing["sun"];
	meshes_glowing["sun"].get_transform().translate(vec3(140, 40, -20));
	meshes_blend["blend_planet"].get_transform().translate(vec3(0, 100, -100));
	meshes_glowing["demon_cube"] = HMesh(geometry_builder::create_box(vec3(0.5f, 0.5f, 0.5f)));
	meshes_glowing["demon_cube"].get_transform().translate(vec3(-43.5f, 0.75f, 40.0f));
	meshes_glowing["demon_cube"].get_transform().rotate(vec3(0.0f, half_pi<float>() / 2.0, 0.0f));
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

	meshes_ground["terrain"].get_material().set_diffuse(vec4(0.2f, 0.2f, 0.2f, 1.0f));
	meshes_ground["terrain"].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes_ground["terrain"].get_material().set_shininess(5.0f);
	meshes_ground["terrain"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
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
}

// Function to load textures
void loadTextures()
{
	glGenVertexArrays(1, &pvao);
	// Load Cubemap
	array<string, 6> filenames = { "textures/purplenebula_ft.tga", "textures/purplenebula_bk.tga", "textures/purplenebula_up.tga",
		"textures/purplenebula_dn.tga", "textures/purplenebula_rt.tga", "textures/purplenebula_lf.tga" };
	cube_map = cubemap(filenames);

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
	texs["ground_heightmap"] = texture("textures/volcano.png");
	texs["sand"] = texture("textures/sand.jpg");
	texs["stone"] = texture("textures/stone.jpg");
	texs["smoke"] = texture("textures/smoke.png");

	// tex_maps points to texturesin texs to reuse textures in some instances
	tex_maps["column1"] = &(texs["gate"]);
	tex_maps["column2"] = &(texs["gate"]);
	tex_maps["gate_ceiling"] = &(texs["gate"]);
	tex_maps["horn1"] = &(texs["gate"]);
	tex_maps["horn2"] = &(texs["gate"]);
	tex_maps["pentagram1"] = &(texs["pentagram"]);
	tex_maps["pentagram2"] = &(texs["pentagram"]);
	tex_maps["lava"] = &(texs["lava"]);
	tex_maps["sun"] = &(texs["sun"]);
	tex_maps["demon_cube"] = &(texs["demon_baby"]);
	tex_maps["ground_heightmap"] = &(texs["ground_heightmap"]);
	tex_maps["smoke"] = &(texs["smoke"]);
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

	sky_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);

	ground_eff.add_shader("shaders/terrain.frag", GL_FRAGMENT_SHADER);
	ground_eff.add_shader("shaders/terrain.vert", GL_VERTEX_SHADER);
	ground_eff.add_shader("shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	ground_eff.add_shader("shaders/part_weighted_texture_4.frag", GL_FRAGMENT_SHADER);

	eff_smoke.add_shader("shaders/smoke.vert", GL_VERTEX_SHADER);
	eff_smoke.add_shader("shaders/smoke.frag", GL_FRAGMENT_SHADER);
	eff_smoke.add_shader("shaders/smoke.geom", GL_GEOMETRY_SHADER);

	compute_eff.add_shader("shaders/particle.comp", GL_COMPUTE_SHADER);

	eff_godraysFirst.add_shader("shaders/spot.vert", GL_VERTEX_SHADER);
	eff_godraysFirst.add_shader("shaders/black.frag", GL_FRAGMENT_SHADER);
	eff_godraysSecond.add_shader("shaders/godRays.vert", GL_VERTEX_SHADER);
	eff_godraysSecond.add_shader("shaders/godRays.frag", GL_FRAGMENT_SHADER);
	// Build effect  
	eff_basic.build();
	eff_phong.build();
	eff_blend.build();
	eff_normal.build();
	eff_glowing.build();
	sky_eff.build();
	ground_eff.build();
	eff_smoke.build();
	compute_eff.build();
	eff_godraysFirst.build();
	eff_godraysSecond.build();

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

// Terrain generation
void generate_terrain(geometry &geom, const texture &height_map, unsigned int width, unsigned int depth,
	float height_scale) {
	// Contains our position data
	vector<vec3> positions;
	// Contains our normal data
	vector<vec3> normals;
	// Contains our texture coordinate data
	vector<vec2> tex_coords;
	// Contains our texture weights
	vector<vec4> tex_weights;
	// Contains our index data
	vector<unsigned int> indices;

	// Extract the texture data from the image
	glBindTexture(GL_TEXTURE_2D, height_map.get_id());
	auto data = new vec4[height_map.get_width() * height_map.get_height()];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void *)data);

	// Determine ratio of height map to geometry
	float width_point = static_cast<float>(width) / static_cast<float>(height_map.get_width());
	float depth_point = static_cast<float>(depth) / static_cast<float>(height_map.get_height());

	// Point to work on
	vec3 point;

	// Part 1 - Iterate through each point, calculate vertex and add to vector
	for (int x = 0; x < height_map.get_width(); ++x) {
		// Calculate x position of point
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (int z = 0; z < height_map.get_height(); ++z) {
			// *********************************
			// Calculate z position of point
			point.z = (depth_point * z) - (depth / 2);
			// *********************************
			// Y position based on red component of height map data
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			// Add point to position data
			positions.push_back(point);
		}
	}

	// Part 1 - Add index data
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x) {
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y) {
			// Get four corners of patch
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			// *********************************
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_height()) + x + 1;
			// *********************************
			// Push back indices for triangle 1 (tl,br,bl)
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			// Push back indices for triangle 2 (tl,tr,br)
			// *********************************
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
			// *********************************
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());

	// Part 2 - Calculate normals for the height map
	for (unsigned int i = 0; i < indices.size() / 3; ++i) {
		// Get indices for the triangle
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];

		// Calculate two sides of the triangle
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];

		// Normal is normal(cross product) of these two sides
		// *********************************
		auto n = normalize(cross(side2, side1));

		// Add to normals in the normal buffer using the indices for the triangle
		normals[idx1] = normals[idx1] + n;
		normals[idx2] = normals[idx2] + n;
		normals[idx3] = normals[idx3] + n;
		// *********************************
	}

	// Normalize all the normals
	for (auto &n : normals) {
		// *********************************
		normalize(n);
		// *********************************
	}

	// Part 3 - Add texture coordinates for geometry
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			tex_coords.push_back(vec2(width_point * x, depth_point * z));
		}
	}

	// Part 4 - Calculate texture weights for each vertex
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			// Calculate tex weight
			vec4 tex_weight(clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.0f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.15f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.5f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.9f) / 0.25f, 0.0f, 1.0f));

			// *********************************
			// Sum the components of the vector
			auto total = tex_weight.x + tex_weight.y + tex_weight.z + tex_weight.w;
			// Divide weight by sum
			tex_weight /= total;
			// Add tex weight to weights
			tex_weights.push_back(tex_weight);
			// ********************************* 
		}
	}

	// Add necessary buffers to the geometry 
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_buffer(tex_weights, BUFFER_INDEXES::TEXTURE_COORDS_1);
	geom.add_index_buffer(indices);

	// Delete data
	delete[] data;
}

void setParticles()
{
	default_random_engine rand(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
	uniform_real_distribution<float> dist(-1.0f, 1.0f );  
	
	for (unsigned int i = 0; i < MAX_PARTICLES; ++i) {
		float randX = dist(rand);
		positions[i] = vec4(randX * 14.0f + meshes_normal["lava"].get_transform().position.x, 30.0f, cos(randX) * 15.0f * dist(rand) + meshes_normal["lava"].get_transform().position.z, 0);
		positions[i].w = positions[i].x;
		velocitys[i] = vec4(0.0f, 10.0f + dist(rand) * 5.0, 0.0f, 0.0f);
	}      
	   
	// a useless vao, but we need it bound or we get errors.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//Generate Position Data buffer
	glGenBuffers(1, &G_Position_buffer);
	// Bind as GL_SHADER_STORAGE_BUFFER
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, G_Position_buffer);
	// Send Data to GPU, use GL_DYNAMIC_DRAW
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(vec4), positions, GL_DYNAMIC_DRAW);

	// Generate Velocity Data buffer
	glGenBuffers(1, &G_Velocity_buffer);
	// Bind as GL_SHADER_STORAGE_BUFFER
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, G_Velocity_buffer);
	// Send Data to GPU, use GL_DYNAMIC_DRAW
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(vec4), velocitys, GL_DYNAMIC_DRAW);
	//Unbind
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void setPostProcess()
{
	//Create Frame buffers
	// Create frame buffer - use screen width and height
	frameGodFirst = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	frameGodTwo = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());

	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);
}

bool load_content() {

	// Set up post processing
	setPostProcess();

	// Create the meshes
	createMeshes();

	// Generate terrain
	geometry geom;
	generate_terrain(geom, texture("textures/volcano.png"), 100, 100, 50.0f);
	meshes_ground["terrain"] = HMesh(geom);

	//Set up materials
	setUpMaterials();

	// Set up Light
	setLight();

	// Load textures
	loadTextures();

	// Load shader and build effects
	loadBuildEffects();

	// Set up particles
	setParticles();

	// Set up cameras
	setCameras();

	return true;
}


bool update(float delta_time) {

	//Set the particle dimensions and movement
	if (delta_time > 10.0f) {
		delta_time = 10.0f;
	}
	renderer::bind(compute_eff);
	glUniform3fv(compute_eff.get_uniform_location("max_dims"), 1, &(vec3(50.0f, 5.0f, 5.0f))[0]);
	glUniform1f(compute_eff.get_uniform_location("delta_time"), delta_time);

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
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_SPACE)) {
			movement = vec3(0.0f, 20.0f, 0.0f) * delta_time;
		}
		if (glfwGetKey(renderer::get_window(), 'Z')) {
			movement = vec3(0.0f, -20.0f, 0.0f) * delta_time;
		}
		if (glfwGetKey(renderer::get_window(), 'Q')) {
			status = (status + 1) % 2;
		}
		if (glfwGetKey(renderer::get_window(), 'E')) {
			toggleGodray = !toggleGodray;
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

vec2 getScreenSpaceSunPos()
{
	auto v = vec4(meshes_glowing["sun"].get_transform().position, 1.0f);
	mat4 V = free_cam.get_view();
	mat4 P = free_cam.get_projection();
	v = V * v;
	v = P * v;
	v /= v.w;
	v += vec4(1.0, 1.0, 0.0, 0.0);
	v /= 2;
	return vec2(v);
}

// SkyBox
void renderSkybox()
{
	// Disable depth test,depth mask,face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(sky_eff);
	// Create MVP Matrix
	mat4 V;
	mat4 P;
	// Calculate MVP for the skybox
	auto M = meshes_skybox["skybox"].get_transform().get_transform_matrix();
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
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(sky_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(sky_eff.get_uniform_location("cubemap"), 0);
	// Render skybox
	renderer::render(meshes_skybox["skybox"]);
	// Enable depth test,depth mask,face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

// FIRST PASS GOD RAYS
void godRaysFirstPass(map<string, HMesh> allMeshes)
{
	// Bind effect
	renderer::bind(eff_godraysFirst);
	for (auto &mesh : allMeshes)
	{
		if (mesh.first == "sun")
			continue;
		auto m = mesh.second;
		mat4 V;
		mat4 P;
		// Calculate MVP for the skybox
		auto M = m.get_transform().get_transform_matrix();
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
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff_godraysFirst.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		renderer::render(m);

	}
}

void godRaysSecondPass()
{
	// Bind effect
	renderer::bind(eff_godraysSecond);
	renderer::set_render_target();

	glUniform2fv(eff_godraysSecond.get_uniform_location("uScreenSpaceSunPos"), 1, value_ptr(getScreenSpaceSunPos()));
	glUniform1f(eff_godraysSecond.get_uniform_location("uDensity"), 1.0f); 
	glUniform1f(eff_godraysSecond.get_uniform_location("uWeight"), 0.01f);
	glUniform1f(eff_godraysSecond.get_uniform_location("uDecay"), 0.99f);  
	glUniform1f(eff_godraysSecond.get_uniform_location("uExposure"), 0.8f);
	glUniform3fv(eff_godraysSecond.get_uniform_location("camViewDirection"), 1, value_ptr(free_cam.get_forward()));
	glUniform3fv(eff_godraysSecond.get_uniform_location("sunViewDirection"), 1, value_ptr(free_cam.get_position() - meshes_glowing["sun"].get_transform().position));
	glUniform1i(eff_godraysSecond.get_uniform_location("status"), status);
	if (toggleGodray)
		glUniform1i(eff_godraysSecond.get_uniform_location("uNumSamples"), 50);
	else
		glUniform1i(eff_godraysSecond.get_uniform_location("uNumSamples"), 0);

	renderer::bind(frameGodFirst.get_frame(), 0);
	// Set the uniform 
	glUniform1i(eff_godraysSecond.get_uniform_location("uOcclusionTexture"), 0);
	 
	renderer::bind(frameGodTwo.get_frame(), 1); 
	// Set the uniform     
	glUniform1i(eff_godraysSecond.get_uniform_location("mainTexture"), 1);

	renderer::render(screen_quad);
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
	auto M = hierarchyCreation(&m);

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

// Ground
void renderHeightGround()
{
	// Bind effect
	renderer::bind(ground_eff);
	// Create MVP matrix
	mat4 M;
	mat4 V;
	mat4 P;
	mat4 MVP;
	vec3 eye_pos;
	if (setFree)
	{
		M = meshes_ground["terrain"].get_transform().get_transform_matrix();
		V = free_cam.get_view();
		P = free_cam.get_projection();
		eye_pos = free_cam.get_position();
		MVP = P * V * M;
	}
	else
	{
		M = meshes_ground["terrain"].get_transform().get_transform_matrix();
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		eye_pos = arc_cam.get_position();
		MVP = P * V * M;
	}
	// Set MVP matrix uniform
	glUniformMatrix4fv(ground_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(ground_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(ground_eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(meshes_ground["terrain"].get_transform().get_normal_matrix()));
	// *********************************
	// Set eye_pos uniform to camera position
	glUniform3fv(ground_eff.get_uniform_location("eye_pos"), 1, value_ptr(eye_pos));
	// *********************************
	//Bind Terrian Material
	renderer::bind(meshes_ground["terrain"].get_material(), "mat");
	// Bind Light
	renderer::bind(light, "light");
	// Bind Tex[0] to TU 0, set uniform
	renderer::bind(texs["sand"], 0);
	glUniform1i(ground_eff.get_uniform_location("tex[0]"), 0);
	// *********************************
	//Bind Tex[1] to TU 1, set uniform
	renderer::bind(texs["stone"], 1);
	glUniform1i(ground_eff.get_uniform_location("tex[1]"), 1);
	// Bind Tex[2] to TU 2, set uniform
	renderer::bind(texs["black_rock"], 2);
	glUniform1i(ground_eff.get_uniform_location("tex[2]"), 2);
	// Bind Tex[3] to TU 3, set uniform
	renderer::bind(texs["black_rock"], 3);
	glUniform1i(ground_eff.get_uniform_location("tex[3]"), 3);
	// *********************************
	// Render terrain
	renderer::render(meshes_ground["terrain"]);
}

void renderParticles()
{
	// Bind Compute Shader
	renderer::bind(compute_eff);
	// Bind data as SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, G_Position_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, G_Velocity_buffer);
	// Dispatch
	glDispatchCompute(MAX_PARTICLES / 128, 1, 1);
	// Sync, wait for completion
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// *********************************
	// Bind render effect
	renderer::bind(eff_smoke);
	// Create MV matrix
	//mat4 M;
	mat4 V;
	mat4 P;
	//mat4 MVP;
	if (setFree)
	{
		//M = meshes_ground["terrain"].get_transform().get_transform_matrix();
		V = free_cam.get_view();
		P = free_cam.get_projection();
		//MVP = P * V * M;
	}
	else
	{
		// = meshes_ground["terrain"].get_transform().get_transform_matrix();
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		//MVP = P * V * M;
	}

	// Set the colour uniform
	glUniform4fv(eff_smoke.get_uniform_location("colour"), 1, value_ptr(vec4(1.0f)));
	// Set MV, and P matrix uniforms seperatly
	glUniformMatrix4fv(eff_smoke.get_uniform_location("MV"), 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(eff_smoke.get_uniform_location("P"), 1, GL_FALSE, value_ptr(P));
	// Set point_size size uniform to .1f
	glUniform1f(eff_smoke.get_uniform_location("point_size"), 1.0f);
	// Bind particle texture
	renderer::bind(texs["smoke"], 0);
	glUniform1i(eff_smoke.get_uniform_location("tex"), 0);
	// *********************************

	// Bind position buffer as GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, G_Position_buffer); 
	// Setup vertex format
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// Enable Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Disable Depth Mask
	glDepthMask(GL_FALSE);
	// Render
	glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
	// Tidy up, enable depth mask
	glDepthMask(GL_TRUE);
	// Disable Blend
	glDisable(GL_BLEND);
	// Unbind all arrays
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

bool render() {

	// Set clear colour to reddish 
	renderer::setClearColour(1.0f, 0.1f, 0.1f);

	// Set render target to frame buffer
	renderer::set_render_target(frameGodFirst);
	// Clear frame
	renderer::clear();
	godRaysFirstPass(meshes_gate);
	godRaysFirstPass(meshes_normal);
	godRaysFirstPass(meshes_phong);
	godRaysFirstPass(meshes_blend);
	godRaysFirstPass(meshes_glowing);
	godRaysFirstPass(meshes_skybox);
	godRaysFirstPass(meshes_ground);

	renderer::set_render_target(frameGodTwo);
	renderer::clear();
	//Render Skybox
	renderSkybox();
	 
	 

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
	// Render ground heightmap
	renderHeightGround();

	// Render Particles
	glBindVertexArray(pvao);
	renderParticles();
	glBindVertexArray(0);

	godRaysSecondPass();
	
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