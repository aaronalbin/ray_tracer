#ifndef CAMERA_H
#define CAMERA_H

#include <Inventor/nodes/SoPerspectiveCamera.h>
 
class Camera
{
private:
	void setCoordSystem();
	void calculateN();
	void calculateU();
	void calculateV();

public:
	SbVec3f n, u, v;
	SbVec3f position;
	SbVec3f rotation_axis;
	float rotation_angle;
	float aspect_ratio;
	SbVec3f direction, view_up;
	float height_angle;

    Camera(); // default constructor
	Camera(const SbVec3f& camera_position,
		const SbVec3f& camera_rotation_axis, 
		const float& camera_rotation_angle, 
		const float& camera_aspect_ratio, 
		const SbVec3f& camera_direction, 
		const SbVec3f& camera_up, 
		const float& camera_height_angle);
	~Camera(); // destructor
};
 
#endif