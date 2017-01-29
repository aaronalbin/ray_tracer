#ifndef LIGHT_H
#define LIGHT_H

#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoPointLight.h>
#include "Color.h"
 
class Light
{
public:
	bool on;
	Color color;
	SbVec3f position;
	float intensity;

    Light(); // default constructor
	Light(const bool& on, const SbVec3f& position, const Color& color, const float& intensity);
	~Light(); // destructor
};
 
#endif