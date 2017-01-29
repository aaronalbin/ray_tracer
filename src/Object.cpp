

#include "Object.h"


Sphere::Sphere(const Color& ambient_color, const Color& diffuse_color, const Color& specular_color, const float& radius, const SbVec3f& center, const float& shininess, const float& transparency,  const SbMatrix& transformation)
{
	this->ambient_color = ambient_color;
	this->diffuse_color = diffuse_color;
	this->specular_color = specular_color;
	this->radius = radius;
	this->center = center;
	this->shininess = shininess;
	this->transparency = transparency;
	this->transformation = transformation;
	this->type = "sphere";
}

float Sphere::intersection(const SbVec3f& rayStart, const SbVec3f& d) const
{
	float t1 = -1;
	float t2 = -1;
	float t = -1;
	float discriminent, disc;
	SbVec3f temp = rayStart - center;   // e - c
	float a = d.dot(d);
	float b = 2*(d.dot(temp));
	float c = temp.dot(temp) - (radius * radius);
	disc = b*b - 4*a*c;
	
	if(disc < 0)
		return 1000000;
	else {
		discriminent = sqrt(disc);
		t1 = (-b+discriminent)/(2*a); 
		t2 = (-b-discriminent)/(2*a);   
		
		if(t1 > 0 && t2 > 0) {
			t = t2;
		}
		else if (t1 > 0 && t2 < 0)
			t = t1;
		else
			t = 1000000;

		return t;
	}
}

float Sphere::shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const
{
	float t1 = -1;
	float t2 = -1;
	float t = -1;
	float discriminent, disc;
	SbVec3f temp = rayStart - center;   // e - c
	float a = rayDirection.dot(rayDirection);
	float b = 2*(rayDirection.dot(temp));
	float c = temp.dot(temp) - (radius * radius);
	disc = b*b - 4*a*c;
	
	if(disc > 0) {
		discriminent = sqrt(disc);
		t1 = (-b+discriminent)/(2*a); 
		t2 = (-b-discriminent)/(2*a);   
		
		if(t1 > 0 && t2 > 0)
			t = 1;
	}

	return t;
}

SbVec3f Sphere::getNormal(const SbVec3f& p) const 
{
	SbVec3f normal = p - this->center;
	normal.normalize();
	return normal;
}


Cube::Cube(const Color& ambient_color, const Color& diffuse_color, const Color& specular_color, const float& width, const float& height, const float& depth, const SbVec3f& center, const float& shininess, const float& transparency, const SbMatrix& transformation)
{
	this->ambient_color = ambient_color;
	this->diffuse_color = diffuse_color;
	this->specular_color = specular_color;
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->center = center;
	this->shininess = shininess;
	this->transparency = transparency;
	this->transformation = transformation;
	this->type = "cube";
}

//d is direction of the ray i.e. (s-e)
float Cube::intersection(const SbVec3f& rayStart, const SbVec3f& d) const
{
	float t1, t2, t_near = -1000000.0f, t_far = 1000000.0f, temp, tCube;
	SbVec3f b1 = getNearPoint();
	SbVec3f b2 = getFarPoint();
	bool intersectFlag = true;

	for (int i = 0; i < 3; i++) {
		if (d[i] == 0) {
			if (rayStart[i] < b1[i] || rayStart[i] > b2[i])
				intersectFlag = false;
		}
		else {
			t1 = (b1[i] - rayStart[i])/d[i];
			t2 = (b2[i] - rayStart[i])/d[i];

			if (t1 > t2) {
				temp = t1;
				t1 = t2;
				t2 = temp;
			}
			if (t1 > t_near)
				t_near = t1;
			if (t2 < t_far)
				t_far = t2;
			if (t_near > t_far)
				intersectFlag = false;
			if (t_far < 0)
				intersectFlag = false;
		}
	}

	if (intersectFlag == false)
		tCube = 1000000;
	else
		tCube = t_near;

	return tCube;
}

float Cube::shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const
{
	return 100000;
}

SbVec3f Cube::getNormal(const SbVec3f& p) const 
{
	float eps = 0.01f;
	SbVec3f normal(0, 0, 0);

	if(abs(p[0] - getNearPoint()[0]) < eps) 
		normal.setValue(-1,0,0);
	else if(abs(p[0] - getFarPoint()[0]) < eps) 
		normal.setValue(1,0,0);
	else if(abs(p[1] - getNearPoint()[1]) < eps) 
		normal.setValue(0,-1,0);
	else if(abs(p[1] - getFarPoint()[1]) < eps) 
		normal.setValue(0,1,0);
	else if(abs(p[2] - getNearPoint()[2]) < eps) 
		normal.setValue(0,0,-1);
	else if(abs(p[2] - getFarPoint()[2]) < eps) 
		normal.setValue(0,0,1);

	return normal;
}

SbVec3f Cube::getFarPoint() const 
{
	return (this->center + SbVec3f(this->width*0.5f, this->height*0.5f, this->depth*0.5f));
}

SbVec3f Cube::getNearPoint() const 
{
	return (this->center - SbVec3f(this->width*0.5f, this->height*0.5f, this->depth*0.5f));
}

Cylinder::Cylinder(const Color& ambient_color, const Color& diffuse_color, const Color& specular_color, const float& radius, const float& height, const SbVec3f& center, const float& shininess, const float& transparency)
{
	this->ambient_color = ambient_color;
	this->diffuse_color = diffuse_color;
	this->specular_color = specular_color;
	this->radius = radius;
	this->height = height;
	this->center = center;
	this->shininess = shininess;
	this->transparency = transparency;
	this->type = "cylinder";
}

float Cylinder::intersection(const SbVec3f& rayStart, const SbVec3f& d) const
{
	float t = 100000;
	float dx = d[0];
	float dz = d[2];
	float ex = rayStart[0];
	float ez = rayStart[2];
	float y_min = this->center[1] - this->height * 0.5;
	float y_max = this->center[1] + this->height * 0.5;

	float a = pow(dx, 2) + pow(dz, 2);
	float b = 2 * (ex * dx + ez * dz);
	float c = pow(ex, 2) + pow(ez, 2) - pow(this->radius, 2);
	float disc = b * b - 4 * a * c;

	if (disc < 0)
	{
		t = 100000;
	}
	else
	{
		float root = sqrt(disc);
		float t1 = (-b+root)/(2*a); 
        float t2 = (-b-root)/(2*a); 
		float y1 = rayStart[1] + t1 * d[1];
		float y2 = rayStart[1] + t2 * d[1];

		if (t2 > 0 && t1 > 0)
		{	
			if((y2 > y_max) || ((y2 < y_min) && (y1 > y_max)) || ((y2 < y_min) && (y1 < y_min)))
				t = 100000;
			else
				t = t2;	
		}
		else if (t1 > 0 && t2 < 0)
		{
			if((t1 < y_min) || (t1 > y_max))
				t = 100000;
			else
				t = t1;
		}
		else
		{
			t = 100000;
		}
	}

	return t;
}

float Cylinder::shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const
{

	return 100000;
}

SbVec3f Cylinder::getNormal(const SbVec3f& p) const 
{
	SbVec3f temp(this->center[0], p[1], this->center[2]);
	
	SbVec3f n = p - temp;
	n.normalize();
	return n;
}
