#version 410

in vec3 dist_of_light;
// the depth map
uniform samplerCube depth_map;
uniform vec3 colour;
out vec4 frag_colour;

float eval_shadow () {
	vec3 v = (dist_of_light);
	float epsilon = 0.0;
	float cubes_dist = textureCube (depth_map, v).r;
	float dist_wor = length (v);

	// ANTON TODO HERE
	// problem is depth map values for distance are 0 to 1
	// and our actual world distances are anything
	//
	// SOLUTION -- linearise depth distance into a real world distance based on
	// near and far clip distances of shadow projection matrix
	
	float n = 1.0f;
	float f = 50.0f;
	float z = (2.0 * n) / (f + n - cubes_dist * (f - n));
	
	//
	// TODO -- get a better number here
	z = z * 30.5;
	
  if (dist_wor < z + epsilon)
      return 1.0; // Inside the light
  else
      return 0.5; // Inside the shadow
}

void main() {
	frag_colour = vec4 (colour, 1.0);
	
	float shadow_factor = eval_shadow ();
	
	frag_colour = vec4 (colour * shadow_factor, 1.0);
}
