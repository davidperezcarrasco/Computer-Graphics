//this var comes from the vertex shader
varying vec2 v_coord;
varying vec3 v_wPos;
varying vec3 v_wNormal;

//the texture passed from the application
uniform sampler2D color_texture;
uniform vec3 light_position;
uniform vec3 eye;
uniform vec3 ia;
uniform vec3 id;
uniform vec3 is;

void main()
{
	
	vec3 l = normalize(light_position-v_wPos);
     vec3 v = normalize(eye-v_wPos);
     vec3 r = reflect(-l,v_wNormal);
	vec3 l_color;
	
	//read the pixel RGBA from the texture at the position v_coord
	vec4 color = texture2D( color_texture, v_coord );
	vec3 k = color.xyz;
	float alpha = 30.0;
	
	vec3 la = k*ia;

     vec3 ld = k*clamp(dot(l,v_wNormal),0.0,1.0)*id;
 
     vec3 ls = k*pow(clamp(dot(r,v),0.0,1.0),alpha)*is;

     l_color = la+ld+ls;
	
	
	//assign the color to the output
	gl_FragColor = vec4(l_color,1.0);
}