#pragma once

#include "framework.h"

//This class contains all the info about the properties of the light
class Light
{
public:

	//you can access this variables directly, do not need a setter/getter

	Vector3 position; //where is the light
	Vector3 diffuse_color; //the amount (and color) of diffuse
	Vector3 specular_color; //the amount (and color) of specular

	Light();

	void addlight(Vector3 p, Vector3 d, Vector3 s);

	//possible method: uploads properties to shader uniforms
	//void uploadToShader(Shader* shader);
};

