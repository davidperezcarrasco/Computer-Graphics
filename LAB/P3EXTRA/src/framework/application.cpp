#include "application.h"
#include "utils.h"
#include "includes.h"
#include "utils.h"

#include "image.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"
#include "light.h"

Material* material = NULL;
Light* light = NULL;


Vector3 ambient_light(0.1, 0.1, 0.1); //here we can store the global ambient light of the scene

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);

	zbuffer.resize(w, h);
	framebuffer.resize(w, h);


}

//Here we have already GL working, so we can create meshes and textures
//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;

	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0, 10, 20), Vector3(0, 10, 0), Vector3(0, 1, 0)); //define eye,center,up
	camera->perspective(60, window_width / (float)window_height, 0.1, 10000); //define fov,aspect,near,far

	//load a mesh
	mesh = new Mesh();
	if (!mesh->loadOBJ("lee.obj"))
		std::cout << "FILE Lee.obj NOT FOUND" << std::endl;

	//load the texture
	texture = new Image();
	texture->loadTGA("color.tga");

	normal_texture = new Image();
	normal_texture->loadTGA("lee_normal.tga");

	//Inicializamos los booleanos de los ejercicios en false
	is_wireframe = false; //Task3:wireframe render
	is_boundingbox = false; //Task4: Triangle with bounding box, with colours RED,GREEN,BLUE
	//create a light (or several) and and some materials
	light = new Light();
	material = new Material();

}

//this function fills the triangle by computing the bounding box of the triangle in screen space and using the barycentric interpolation
//to check which pixels are inside the triangle. It is slow for big triangles, but faster for small triangles 
void Application::fillTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector2& uv0, const Vector2& uv1, const Vector2& uv2)
{
	//compute triangle bounding box in screen space
	framebuffer.initTable();
	framebuffer.fillTable(p0.x, p0.y, p1.x, p1.y);
	framebuffer.fillTable(p1.x, p1.y, p2.x, p2.y);
	framebuffer.fillTable(p2.x, p2.y, p0.x, p0.y);
	
	if (is_wireframe) { //si está activa la tarea 3 simplemente dibujamos las 3 líneas blancas(el algoritmo bresenham ya comprueba que los 2 vértices de cada línea estén dentro del framebuffer)
		framebuffer.Bresenham(p0.x, p0.y, p1.x, p1.y, Color::WHITE);
		framebuffer.Bresenham(p1.x, p1.y, p2.x, p2.y, Color::WHITE);
		framebuffer.Bresenham(p2.x, p2.y, p0.x, p0.y, Color::WHITE);
	}
	//Creamos los vectores entre P0 y P1 y entre P0 y P2
	Vector3 v0( p1.x - p0.x, p1.y - p0.y, p1.z-p0.z);
	Vector3 v1(p2.x - p0.x, p2.y - p0.y,p2.z-p0.z);

	//Calculamos los productos escalares y el denominador
	float d00 = v0.dot(v0);
	float d01 = v0.dot(v1);
	float d11 = v1.dot(v1);
	float denom = d00 * d11 - d01 * d01;
	//clamp to screen area
	

	//loop all pixels inside bounding box
	for (int y = 0; y < framebuffer.height; y++) {
		for (int x = framebuffer.table[y].minx ; x <= framebuffer.table[y].maxx ; x++) { //dejamos un margen de unos 2 píxeles para que se pinte el contorno(podríamos hacer bresenham entre los vértices pero no estarían interpolados)
			if (x > 0 && y > 0 && x < framebuffer.width && y < framebuffer.height) { //Comprobamos para cada punto que se encuentre dentro del framebuffer

				//we must compute the barycentrinc interpolation coefficients, weights of pixel P(x,y)
				Vector3 v2(x - p0.x, y - p0.y,1); //Calculamos el vector y los productos escalares entre los vértices y cada punto

				float d20 = v2.dot(v0);
				float d21 = v2.dot(v1);

				float v = clamp((d11 * d20 - d01 * d21) / denom,0,1); //Calculamos los pesos correspondientes a cada color
				float w = clamp((d00 * d21 - d01 * d20) / denom,0,1);
				float u = 1.0 - v - w;

				//check if pixel is inside or outside the triangle
				if (v >= -0.5 && v <= 1.5 && w >= -0.5 && w <= 1.5 && u >= -0.5 && u <= 1.5) { 
					//here add your code to test occlusions based on the Z of the vertices and the pixel (TASK 5)
					float z = p0.z * u + p1.z * v + p2.z * w; //Calculamos la nueva z
					

					if (z < zbuffer.getPixel(x, y)) { //Si el punto en cuestión está más cerca que los otros en las mismas coords lo dibujaremos, sino lo saltamos
						//here add your code to compute the color of the pixel (barycentric interpolation) (TASK 4)
						zbuffer.setPixel(x, y, z); //introducimos el nuevo valor z
						int texx = clamp((uv0.x * u + uv1.x * v + uv2.x * w), 0.0, 1.0) * texture->width; //Addquirimos las coordenadas x e y multiplicando el coeficiente de interpolación de cada una por el ancho o largo de la iamgen original
						int texy = clamp((uv0.y * u + uv1.y * v + uv2.y * w), 0.0, 1.0) * texture->height;

						int ntexx = clamp((uv0.x * u + uv1.x * v + uv2.x * w),0.0,1.0) * normal_texture->width; //Consideramos las dimensiones de la textura normal
						int ntexy = clamp((uv0.y * u + uv1.y * v + uv2.y * w),0.0,1.0) * normal_texture->height;

						Color n = normal_texture->getPixel(ntexx, ntexy); //Obtenemos el color de la textura
						Vector3 normal = Vector3(n.r, n.g, n.b)/255.0; //Pasamos el color a vector 0..1
						//normal.normalize();

						Color c = texture->getPixel(texx, texy); //Obtenemos el nuevo color
						Vector3 k = Vector3((float)c.r/255.0,(float) c.g/255.0,(float) c.b/255.0 ); //Pasamos el color a 0..1
						//k.normalize();

						Vector3 pos = Vector3(x, y, z);
						Vector3 l = light->position - pos;
						l.normalize();

						Vector3 view = camera->eye - pos;
						view.normalize();

						Vector3 l2 = pos - light->position;
						l2.normalize();

						Vector3 r = framebuffer.reflect(l2, normal);
						r.normalize();

						Vector3 la = Vector3(ambient_light.x * material->ambient.x * k.x, ambient_light.y * material->ambient.y * k.y, ambient_light.z * material->ambient.z * k.z);

						Vector3 ld = Vector3(material->diffuse.x*k.x * light->diffuse_color.x, material->diffuse.y * light->diffuse_color.y*k.y, material->diffuse.z * light->diffuse_color.z*k.z) * max(float(normal.dot(l2)), float(0));
			
						Vector3 ls = Vector3(material->specular.x * light->specular_color.x*k.x, material->specular.y * light->specular_color.y*k.y, material->specular.z * light->specular_color.z*k.z) * pow(max((float)r.dot(view), float(0)), material->shininess);
						
						
						Vector3 lp = la+ld+ls;
			
						//volvemos a pasar a 255
						c.r = 255*lp.x;
						c.g =  255*lp.y;
						c.b =  255*lp.z;
						
						if (is_boundingbox) { //Si es la tarea4 el color será elr esultado de la interpolación entre rojo, verde y azul
							c = Color::RED * u + Color::GREEN * v + Color::BLUE * w;
						}
						//draw the pixels in the colorbuffer x,y 
						if (!is_wireframe) { //si no es la tarea3 dibujaremos el punto
							framebuffer.setPixel(x, y, c);
						}
					}
				}
			}
		}
	}
}
     

//render one frame
void Application::render(Image& framebuffer)
{
	//
	framebuffer.fill(Color(40, 45, 60)); //clear
	zbuffer.fill(10000);
	
	//for every point of the mesh (to draw triangles take three points each time and connect the points between them (1,2,3,   4,5,6,   ...)
	for (int i = 0; i < mesh->vertices.size()-3; i+=3)
	{

	
		Vector3 vertex = mesh->vertices[i]; //extract vertex from mesh
		Vector2 texcoord = mesh->uvs[i]; //texture coordinate of the vertex (they are normalized, from 0,0 to 1,1)
		Vector3 normal = mesh->normals[i]; //Obtenemos las normales

		Vector3 vertex2 = mesh->vertices[i+1]; 
		Vector2 texcoord2 = mesh->uvs[i+1];
		Vector3 normal2 = mesh->normals[i+1];

		Vector3 vertex3 = mesh->vertices[i + 2];
		Vector2 texcoord3 = mesh->uvs[i + 2];
		Vector3 normal3 = mesh->normals[i+2];


		//project every point in the mesh to normalized coordinates using the viewprojection_matrix inside camera
		Vector3 normalized_point = camera->projectVector( vertex );
		Vector3 normalized_point2 = camera->projectVector(vertex2);
		Vector3 normalized_point3 = camera->projectVector(vertex3);

		//Pasamos de coordenadas noramlizadas a las coordenadas del framebuffer
		normalized_point.x = (normalized_point.x + 1) * framebuffer.width / 2;
		normalized_point.y = (normalized_point.y + 1) * framebuffer.height / 2;

		normalized_point2.x = (normalized_point2.x + 1) * framebuffer.width / 2;
		normalized_point2.y = (normalized_point2.y + 1) * framebuffer.height / 2;

		normalized_point3.x = (normalized_point3.x + 1) * framebuffer.width / 2;
		normalized_point3.y = (normalized_point3.y + 1) * framebuffer.height / 2;

		//paint points in framebuffer (using your drawTriangle function or the fillTriangle function)
		fillTriangle(normalized_point, normalized_point2, normalized_point3, texcoord, texcoord2, texcoord3);
		
	}
}

//called after render
void Application::update(double seconds_elapsed)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	if (keystate[SDL_SCANCODE_SPACE])
	{
		//...
	}
	
	//example to move eye
	if (keystate[SDL_SCANCODE_LEFT] && camera->eye.x-5*seconds_elapsed>-(framebuffer.width*0.5)) //creamos unos máximos para que el eye no se aleje del exterior de la pantalla(como mmáximo que desaparezca por unos píxeles)
		camera->eye.x -= 5 * seconds_elapsed;

	if (keystate[SDL_SCANCODE_RIGHT] && camera->eye.x + 5 * seconds_elapsed < framebuffer.width*0.5)
		camera->eye.x += 5 * seconds_elapsed;

	if (keystate[SDL_SCANCODE_UP] && camera->eye.y + 5 * seconds_elapsed < framebuffer.height*0.5)
		camera->eye.y += 5 * seconds_elapsed;

	if (keystate[SDL_SCANCODE_DOWN] && camera->eye.y - 5 * seconds_elapsed > -(framebuffer.height*0.5))
		camera->eye.y -= 5 * seconds_elapsed;

	if (keystate[SDL_SCANCODE_A] && camera->center.x+5*seconds_elapsed<40)//creamos unos máximos para que el center no se aleje del exterior de la pantalla(como mmáximo que desaparezca por unos píxeles)
		camera->center.x +=  5* seconds_elapsed;

	if (keystate[SDL_SCANCODE_D]&& camera->center.x-5*seconds_elapsed > -40)
		camera->center.x -= 5* seconds_elapsed;

	if (keystate[SDL_SCANCODE_W] && camera->center.y - 5 * seconds_elapsed > -20)
		camera->center.y -= 5* seconds_elapsed;

	if (keystate[SDL_SCANCODE_S] && camera->center.y + 5 * seconds_elapsed < 40)
		camera->center.y += 5* seconds_elapsed;

	if (keystate[SDL_SCANCODE_F] && camera->fov-5*seconds_elapsed>0) //evitamos que el fov llegue a 0 e invierta la iamgen con los valores negativos
		camera->fov -= 5 * seconds_elapsed;

	if (keystate[SDL_SCANCODE_G]&&camera->fov+5*seconds_elapsed<180) //evitamos que el fov llegue a 180 e invierta la imagen
		camera->fov += 5 * seconds_elapsed;


	//if we modify the camera fields, then update matrices
	camera->updateViewMatrix();
	camera->updateProjectionMatrix();
}

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
		case SDL_SCANCODE_ESCAPE: exit(0); break; //ESC key, kill the app
	}

	if (event.keysym.scancode == SDL_SCANCODE_3) { //Con la tecla 3 activamos el modo wireframe render(triángulos blancos sin zbuffer)
		if (is_wireframe) {
			is_wireframe = false;
		}
		else {
			is_wireframe = true;
			if (is_boundingbox) is_boundingbox = false; //si estaba activada la task4 la desactivamos (no mezclaremos el wireframe render blanco con los triángulos interpolados)
		}
	}

	if (event.keysym.scancode == SDL_SCANCODE_4) { //Con la tecla 4 activamos los triángulos interpolados de rojo azul y verde
		if (is_boundingbox) {
			is_boundingbox = false;
		}
		else {
			is_boundingbox = true; 
			if (is_wireframe) is_wireframe = false; //desactivamos los triángulos blancos(wireframe render task3)
		}
	}
}

//keyboard released event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
	}
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{

	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
