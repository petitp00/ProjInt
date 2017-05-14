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
	float slope1 = 5000; // bigger = dist is more linear
	float max1 = 50; // value at the center
	float dist1 = min(max1, slope1/(d+slope1/max1));
	float slope2 = 4; // bigger = ripples fades out further
	float max2 = 1; // max value of the multiplier
	float dist2 = min(max2, slope2/(d+slope2/max2));

	float time1 = 4 * t;

	float tslope2 = 0.04;
	float speed = 20;
	float time2 = max(0, -pow(tslope2*(d-t*speed), 2) + 1);

	float val = (0.5 + 0.5 * -cos(dist1 + time1)) * (dist2) * time2 * max(0, min(1, -5*t+15));
	return val;

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
			float duration = r.z;
			float start_time = r.w;
			float time_elapsed = time - start_time;

			if (duration != 0 && time_elapsed < duration) {
				float dist = sqrt(pow(r.x - vert_pos.x, 2) + pow(r.y - vert_pos.y, 2));

				float val = rip_cos(time_elapsed, dist);

				opt2 += val; 
			}
		}

		color = vec4(blendMult(vec3(pixel), vec3(water_color), 1.0), 1.0); // color of the water
		opt += opt2;
		color += vec4(opt, opt, opt, 0); // add the color of the highlights
	}

    gl_FragColor = color;
}
