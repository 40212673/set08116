#version 450 core


vec3 godrays(float density, float weight, float decay, float exposure, int numSamples, sampler2D occlusionTexture, sampler2D mainTexture, vec2 screenSpaceLightPos, vec2 uv) {

    vec3 fragColor = vec3(0.0,0.0,0.0);

	vec2 deltaTextCoord = vec2( uv - screenSpaceLightPos.xy );

	vec2 textCoo = uv.xy ;
	deltaTextCoord *= (1.0 /  float(numSamples)) * density;
	float illuminationDecay = 1.0;


	for(int i=0; i < 100 ; i++){

	    if(numSamples < i) {
            break;
	    }

		textCoo -= deltaTextCoord; 
		vec3 samp = texture2D(occlusionTexture, textCoo   ).xyz;
		samp *= illuminationDecay * weight;
		fragColor += samp;
		illuminationDecay *= decay;
	}

	fragColor *= exposure;
	vec4 main_sample = texture(mainTexture, uv);
	fragColor = fragColor + main_sample.xyz;
    return fragColor;
}

uniform sampler2D mainTexture;
uniform sampler2D uOcclusionTexture;
layout(location = 1) in vec2 vUv;
layout(location = 0) out vec4 colour;

uniform vec2 uScreenSpaceSunPos;

uniform float uDensity;
uniform float uWeight;
uniform float uDecay;
uniform float uExposure;
uniform int uNumSamples;


void main() {

	vec3 fragColor = godrays(uDensity, uWeight, uDecay, uExposure, uNumSamples, uOcclusionTexture, mainTexture, uScreenSpaceSunPos, vUv);

    colour = vec4(fragColor , 1.0);
}