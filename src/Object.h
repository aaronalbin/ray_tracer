#ifndef OBJECT_H_
#define OBJECT_H_

#include <iostream>
#include "OSUInventor.h"
#include "Color.h"

class Object {

public:
	float shininess, transparency;
	Color ambient_color, diffuse_color, specular_color;
	SbVec3f center;
	std::string type;
	SbMatrix transformation;

	virtual float intersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const = 0;
	virtual float shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const = 0;
	virtual SbVec3f getNormal(const SbVec3f& p) const = 0;

	virtual ~Object() {}
};

class Sphere : public Object {
public:
	float radius;

	Sphere(const Color& ambient_color, const Color& diffuse_color, const Color& specular_color, const float& radius, const SbVec3f& center, const float& shininess, const float& transparency, const SbMatrix& transformation);

	virtual float intersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const;
	virtual float shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const;
	virtual SbVec3f getNormal(const SbVec3f& p) const;

	~Sphere() {}
};

class Cube : public Object {
public:
	float height, width, depth;

	Cube(const Color& ambient_color, const Color& diffuse_color, const Color& specular_color, const float& width, const float& height, const float& depth, const SbVec3f& center, const float& shininess, const float& transparency, const SbMatrix& transformation);

	virtual float intersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const;
	virtual float shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const;
	virtual SbVec3f getNormal(const SbVec3f& p) const;

	SbVec3f getNearPoint() const;
	SbVec3f getFarPoint() const;

	~Cube() {}
};

class Cylinder : public Object {

public:
	float radius, height;

	Cylinder(const Color& ambient_color, const Color& diffuse_color, const Color& specular_color, const float& radius, const float& height, const SbVec3f& center, const float& shininess, const float& transparency);

	virtual float intersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const;
	virtual float shadowIntersection(const SbVec3f& rayStart, const SbVec3f& rayDirection) const;
	virtual SbVec3f getNormal(const SbVec3f& p) const;
};

#endif

