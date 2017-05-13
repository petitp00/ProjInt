uniform sampler2D texture;
uniform float time;
uniform vec4 ripples[16];

in vec2 vert_pos;

float PI = 3.14159;

vec3 blendMult(vec3 base, vec3 blend) {
	return base*blend;
}

vec3 blendMult(vec3 base, vec3 blend, float opacity) {
	return (blendMult(base, blend) * opacity + base * (1.0 - opacity));
}

float rip_cos(float t, float d) {
	// min: 0.2
	// max: 0.8
	// start 0.8
	return 0.3 * cos(0.15 * (t - 100.0*d)) + 0.5;
}

float sinb(float x) { // returns between 0 and 1
	return 0.6 + 0.4 * sin(0.1 * x);
}

float wat1(float t, float x, float y) {
	float res = sinb(t * 10 + 0.6*x + 0.2*y) + 0.3 * sinb(t * 20 - 0.2 * x + 0.8 * y) + 0.8 * sinb(t * 25 + x - 0.7 * y) + 0.2*sinb(t * 5 + x);
	res /= 1.5; 
	return res;
}
float wat2(float t, float x, float y) {
	float res = sinb(t * 25 - 1.2 * x + y) + 0.7 * sinb(t * 15 + 0.2 * x - 0.6 * -y) + 0.4 * sinb(t * 9 + 0.1 * x + y) + 0.6 * sinb(t * 20 + 2 * y);
	res /= 1.5;
	return res;
}

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

	vec4 water_color = vec4(116.0/255.0, 196.0/255.0, 194.0/255.0, 255.0/255.0);
	vec4 color = pixel;

	if (gl_Color.r == 0.0) {
		float op1 = 1.0 + 0.5 * sin(time*1.2 + (vert_pos.x*0.1 + vert_pos.y*0.05));
		float op2 = 1.0 + 0.5 * cos(time*0.8 + (vert_pos.x*-0.2 + vert_pos.y*0.1));
		op1 = wat1(time, vert_pos.x, vert_pos.y);
		op2 = wat2(time, vert_pos.x, vert_pos.y);
		float opt = op1 * op2;
		opt *= 0.2;

		float opt2 = 0;
		for (int i = 0; i != 16; i++) {
			vec4 r = ripples[i];
			if (ripples[i].z != 0) {
				float dist = sqrt(pow(r.x - vert_pos.x, 2) + pow(r.y - vert_pos.y, 2));
				float t = r.z/100.f;

				float max_dist = 35.f + r.w * 2;
				float dist2 = (max_dist - dist)/max_dist; // goes from 0 to 1, 1 is the closest to the point
				if (dist2 < 0) dist2 = 0;

				opt2 += dist2*0.5 * rip_cos(t, dist2) * min(1, t/10);
			}
		}

		color = vec4(blendMult(vec3(pixel), vec3(water_color), 1.0), 1.0); // color of the water
		opt += opt2;
		color += vec4(opt, opt, opt, 0); // add the color of the highlights
	}

    gl_FragColor = color;
}
