#version 430 core

// Incoming frame data
uniform sampler2D tex;

// 1.0f / screen width
uniform float inverse_width;
// 1.0f / screen height
uniform float inverse_height;

// Surrounding pixels to sample and their scale
const vec4 samples[4] = vec4[4](vec4(-1.0, 0.0, 0.0, 0.25), vec4(1.0, 0.0, 0.0, 0.25), vec4(0.0, 1.0, 0.0, 0.25),
                                vec4(0.0, -1.0, 0.0, 0.25));

// Edge detection
const vec4 edgeDetection[6] = vec4[6](vec4(-1.0, 1.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 2.0), vec4(1.0, 1.0, 0.0, 1.0),
										vec4(-1.0, -1.0, 0.0, -1.0), vec4(0.0, -1.0, 0.0, -2.0), vec4(1.0, -1.0, 0.0, -1.0));

// Sharpening filter
const vec4 sharpFilter[5] = vec4[5](vec4(0.0, 1.0, 0.0, -2.0/3.0), vec4(-1.0, 0.0, 0.0, -2.0/3.0), vec4(0.0, 0.0, 0.0, 11.0/3.0),
									vec4(1.0, 0.0, 0.0, -2.0/3.0), vec4(0.0, -1.0, 0.0, -2.0/3.0));

// Gaussian blur 1
const vec4 gauBlurA[7] = vec4[7](vec4(-3.0, 0.0, 0.0, 1.0/64.0), vec4(-2.0, 0.0, 0.0, 6.0/64.0), vec4(-1.0, 0.0, 0.0, 15.0/64.0),
									vec4(0.0, 0.0, 0.0, 20.0/64.0), vec4(1.0, 0.0, 0.0, 15.0/64.0), vec4(2.0, 0.0, 0.0, 6.0/64.0), vec4(3.0, 0.0, 0.0, 1.0/64.0));

// Gaussian blur 2
const vec4 gauBlurB[7] = vec4[7](vec4(0.0, -3.0, 0.0, 1.0/64.0), vec4( 0.0, -2.0, 0.0, 6.0/64.0), vec4(0.0, -1.0, 0.0, 15.0/64.0),
									vec4(0.0, 0.0, 0.0, 20.0/64.0), vec4(0.0, 1.0, 0.0, 15.0/64.0), vec4(0.0, 2.0, 0.0, 6.0/64.0), vec4(0.0, 3.0, 0.0, 1.0/64.0));

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Start with colour as black
  vec4 start_colour = vec4(0.0, 0.0, 0.0, 1.0);
  // Loop through each sample vector
  for (int i = 0; i < 7; i++)
  {
    // Calculate tex coord to sample
	vec2 uv = tex_coord + vec2(gauBlurA[i].x * inverse_width, gauBlurA[i].y * inverse_height);
    // Sample the texture and scale appropriately
    // - scale factor stored in w component
	vec4 sample_tex = texture(tex, uv) * gauBlurA[i].w; 
	start_colour += sample_tex;
  }

  for (int i = 0; i < 7; i++)
  {
    // Calculate tex coord to sample
	vec2 uv = tex_coord + vec2(gauBlurB[i].y *inverse_height , gauBlurB[i].x * inverse_width );
    // Sample the texture and scale appropriately
    // - scale factor stored in w component
	vec4 sample_tex = texture(tex, uv) * gauBlurB[i].w; 
	start_colour += sample_tex;
  }
  // Ensure alpha is 1.0
  colour = start_colour;
  colour.a = 1.0;
  // *********************************
}