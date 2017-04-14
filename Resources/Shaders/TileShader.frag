uniform sampler2D texture;
uniform float time;

in vec2 vert_pos;

vec3 blendMult(vec3 base, vec3 blend) {
	return base*blend;
}

vec3 blendMult(vec3 base, vec3 blend, float opacity) {
	return (blendMult(base, blend) * opacity + base * (1.0 - opacity));
}

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

	//vec4 water_color = vec4(66.0/255.0, 203.0/255.0, 244.0/255.0, 255.0/255.0);
	vec4 water_color = vec4(116.0/255.0, 196.0/255.0, 194.0/255.0, 255.0/255.0);
	vec4 color = pixel;

	if (gl_Color.r == 0.0) {
		float op1 = 1.0 + 0.5 * sin(time*1.2 + (vert_pos.x*0.1 + vert_pos.y*0.05));
		float op2 = 1.0 + 0.5 * cos(time*0.8 + (vert_pos.x*-0.2 + vert_pos.y*0.1));
		float opt = op1 * op2;
		opt *= 0.1;

		color = vec4(blendMult(vec3(pixel), vec3(water_color), 1.0), 1.0);
		color += vec4(opt, opt, opt, 0);
	}

    // multiply it by the color
    gl_FragColor = color;// * pixel;
}
