
#include "Light.h"
 
Light::Light() 
{}

Light::Light(const bool& on, const SbVec3f& position, const Color& color, const float& intensity)
{
	this->on = on;
	this->position = position;
	this->color = color;
	this->intensity = intensity;
}

Light::~Light()
{}