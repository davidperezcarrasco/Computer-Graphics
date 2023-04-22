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
uniform vec3 ka;
uniform vec3 ks;
uniform vec3 kd;
uniform float alpha;

void main()
{
	vec3 N = normalize(v_wNormal);
	
	vec3 l = normalize(light_position-v_wPos);
     vec3 v = normalize(eye-v_wPos);
     vec3 r = reflect(-l,N);
	vec3 l_color;
	
	//read the pixel RGBA from the texture at the position v_coord
	vec4 color = texture2D( color_texture, v_coord );
	color.xyz = color.xyz; // Change material properties
	vec3 k = color.xyz;
	vec3 kss = k*color.w;
	
	vec3 la = k*ia*ka;

     vec3 ld = kd*k*clamp(dot(l,N),0.0,1.0)*id;
 
     vec3 ls = ks*kss*pow(clamp(dot(r,v),0.0,1.0),alpha)*is;

     l_color = la+ld+ls;
	
	
	//assign the color to the output
	gl_FragColor = vec4(l_color,1.0);
}