#include "application.h"
#include "utils.h"
#include "image.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "material.h"


#define MAX_MESHES 50
#define MAX_LIGHTS 5

Camera* camera = NULL;
Mesh* mesh = NULL;
Matrix44 model_matrix;
Shader* shader = NULL;
Shader* task1 = NULL;
Shader* task2 = NULL;
Shader* task3 = NULL;
Texture* texture = NULL;
Texture* normal = NULL; 
Light* light = NULL;

float angle = 0;
int shaderSwap = 1;
int N_MESHES = 1; // Número de meshes
int N_ENTITIES = 1; // Número de entidades
int LIGHTS = 0; // Luces activas

Vector3 ambient_light(0.1, 0.1, 0.1);

Mesh* meshes[MAX_MESHES]; // Distintos meshes
Light* lights[MAX_LIGHTS]; // Distintas luces


Application::Application(const char* caption, int width, int height)
{
	
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,20,20),Vector3(0,10,0),Vector3(0,1,0));
	camera->setPerspective(60,window_width / window_height,0.1,10000);

	//then we load a mesh
	meshes[0] = new Mesh();
	meshes[0]->loadOBJ("../res/meshes/lee.obj");

	//load the texture
	texture = new Texture();
	if(!texture->load("../res/textures/lee_color_specular.tga"))
	{
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}
	normal = new Texture();
	if (!normal->load("../res/textures/lee_normal.tga"))
	{
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}


	//we load a shader
	task1 = Shader::Get("../res/shaders/texture_1.vs","../res/shaders/texture_1.fs");
	task2 = Shader::Get("../res/shaders/texture_2.vs", "../res/shaders/texture_2.fs");
	task3 = Shader::Get("../res/shaders/texture_3.vs", "../res/shaders/texture_3.fs");
	shader = task1;
	//load whatever you need here
	//......
	lights[0] = new Light();

	entities[0].material = new Material();

	entities[0].material->ambient = Vector3(1.0, 1.0, 1.0);
	entities[0].material->diffuse = Vector3(1, 1, 1);
	entities[0].material->specular = Vector3(1, 1, 1);
	entities[0].material->shininess = 30.0;
	
	entities[0].mesh = meshes[0];

}

//render one frame
void Application::render(void)
{
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST );

	//Get the viewprojection
	camera->aspect = window_width / window_height;
	Matrix44 viewprojection = camera->getViewProjectionMatrix();
	
	//enable the shader
	shader->enable();
	//shader->setMatrix44("model", model_matrix); //upload info to the shader
	//shader->setMatrix44("viewprojection", viewprojection); //upload info to the shader

	shader->setTexture("color_texture", texture, 0 ); //set texture in slot 0
	shader->setTexture("normal_texture", normal, 1); //set texture in slot 1

	shader->setUniform3("eye", camera->eye); 
	shader->setUniform3("ia", ambient_light);

	shader->setUniform3("light_position", lights[LIGHTS]->position);
	shader->setUniform3("id", lights[LIGHTS]->diffuse_color);
	shader->setUniform3("is", lights[LIGHTS]->specular_color);


	//render the data
	for (int i = 0; i < N_ENTITIES; i++) { // Itera tantas veces como meshe


		shader->setMatrix44("model", entities[i].matrix); //upload the transform matrix to the shader
		shader->setMatrix44("viewprojection", viewprojection); //upload viewprojection info to the shader
		shader->setUniform3("ka", entities[i].material->ambient);
		shader->setUniform3("ks", entities[i].material->specular);
		shader->setUniform3("kd", entities[i].material->diffuse);
		shader->setUniform1("alpha", entities[i].material->shininess);
		entities[i].mesh->render(GL_TRIANGLES); // Render de todas las entidades con su material y posición
	}

	//disable shader
	shader->disable();

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//called after render
void Application::update(double seconds_elapsed)
{
	//NO BORRAR
	if (keystate[SDL_SCANCODE_SPACE])
	{
		model_matrix.rotateLocal(seconds_elapsed,Vector3(0,1,0));
	}

	if (keystate[SDL_SCANCODE_A]) 
		camera->orbit(seconds_elapsed * 2, Vector3(0, 1, 0));
	else if (keystate[SDL_SCANCODE_D])
		camera->orbit(seconds_elapsed * 2, Vector3(0, -1, 0));
	if (keystate[SDL_SCANCODE_W])
		camera->orbit(seconds_elapsed * 2, Vector3(1, 0, 0));
	else if (keystate[SDL_SCANCODE_S])
		camera->orbit(seconds_elapsed * 2, Vector3(-1, 0, 0));

	if (keystate[SDL_SCANCODE_RIGHT]) // Luz aumenta x 
		lights[LIGHTS]->position = lights[LIGHTS]->position + Vector3(1, 0, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_LEFT]) // Luz disminuye x
		lights[LIGHTS]->position = lights[LIGHTS]->position + Vector3(-1, 0, 0) * seconds_elapsed * 10.0;
	if (keystate[SDL_SCANCODE_UP]) // Luz aumenta y
		lights[LIGHTS]->position = lights[LIGHTS]->position + Vector3(0, 1, 0) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_DOWN]) // Luz disminuye y
		lights[LIGHTS]->position = lights[LIGHTS]->position + Vector3(0, -1, 0) * seconds_elapsed * 10.0;
	if (keystate[SDL_SCANCODE_K]) // Luz aumenta z
		lights[LIGHTS]->position = lights[LIGHTS]->position + Vector3(0, 0, 1) * seconds_elapsed * 10.0;
	else if (keystate[SDL_SCANCODE_L]) // Luz disminuye z
		lights[LIGHTS]->position = lights[LIGHTS]->position + Vector3(0, 0, -1) * seconds_elapsed * 10.0;


	for (int i = 0; i < N_ENTITIES; i++) { // Itera tantas veces como meshes

		entities[i].matrix.setIdentity();
		if (i % 2 != 0) { // Se colocan a ambos lados del mesh central
			entities[i].matrix.translate(6 * -i - 12, 0, 8 * i + 8); // La ubica a una distancia constante en función del N_ENTITIES activos
		}
		else {
			entities[i].matrix.translate(6 * i, 0, -8 * i);
		}
	}
}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode){
		case SDL_SCANCODE_R: Shader::ReloadAll(); break;
	
		case SDL_SCANCODE_1:
			if (shader != task1) {
				shader = task1;
				printf("Shader mode: Task 1.\n");
			}
			break;
		case SDL_SCANCODE_2:
			if (shader != task2) {
				shader = task2;
				printf("Shader mode: Task 2.\n");
			}
			break;
		case SDL_SCANCODE_3:
			if (shader != task3) {
				shader = task3;
				printf("Shader mode: Task 3.\n");
			}
			break;
		case SDL_SCANCODE_Z: // Añade meshes
			if (N_ENTITIES < MAX_ENTITIES) {

				// Materiales "aleatorios"
				int a = N_ENTITIES % 7;
				int b = N_ENTITIES % 6;
				int c = N_ENTITIES % 5;


				entities[N_ENTITIES].material = new Material();
				// Uniforms para los shaderss
				entities[N_ENTITIES].material->ambient = Vector3(1.0 / 6.0 * a, 1.0 / 5.0 * b, 1.0 / 4.0 * c);
				entities[N_ENTITIES].material->diffuse = Vector3(1.0 / 6.0 * b, 1.0 / 5.0 * c, 1.0 / 4.0 * a);
				entities[N_ENTITIES].material->specular = Vector3(1.0 / 6.0 * c, 1.0 / 5.0 * a, 1.0 / 4.0 * b);
				entities[N_ENTITIES].material->shininess = 2 + a * b;
				
				entities[N_ENTITIES].mesh = meshes[0];
				N_ENTITIES++;
			    printf("Number of entitites: %d/%d.\n", N_ENTITIES, MAX_ENTITIES);
			}
			else
			{
				printf("Max number of entities reached: %d/%d.\n", N_ENTITIES, MAX_ENTITIES);
			}
			break; //ESC key, kill the app
		case SDL_SCANCODE_X: // Quita meshes
			if (N_ENTITIES > 1) {
				N_ENTITIES--;
				printf("Number of entities: %d/%d.\n", N_ENTITIES, MAX_ENTITIES);
			}
			else
			{
				printf("Min number of meshes reached: %d/%d.\n", N_ENTITIES, MAX_ENTITIES);
			}
			break; //ESC key, kill the app
        case SDL_SCANCODE_ESCAPE: exit(0); break; //ESC key, kill the app
		
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
