#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include <vector>
#include <time.h>
#include "Camera.h"
#include "Object.h"
#include "Light.h"

class RayTracer
{
private:
	Color shade(const SbVec3f& startingRay, const SbVec3f& rayDirection, const int& recursionDepth, const int& mainx, const int& mainy);
	Color threshold(const Color& c); 
	bool intersectShadowRay(const SbVec3f& p, const SbVec3f& l, const SbVec3f normal);
	void writeToImage();
	Color checkers(const float& x, const float& mainy, const float& sx, const float& sy);
	Color grain(const float& x, const float& mainy, const float& sx, const float& sy);

	int x, y, MAX_RECURSION, shadow_flag, reflection_on, refraction_on, depth_of_field_on, focal_length, dof_num_rays, soft_shadow_num_rays, finalscene;
	char* out_filename;
	char* in_filename;
	std::vector<Color> pixels;
	std::vector<Object*> objects;
	std::vector<Sphere> spheres;
	std::vector<Cube> cubes;
	std::vector<Cylinder> cylinders;
	std::vector<Light> lights;
	Camera cam;
	time_t seconds;

public:
	RayTracer(); 
	~RayTracer();
	void read_scene(int argc, char **argv);
	void trace_rays();
	float getRandomJitter();
};

#endif