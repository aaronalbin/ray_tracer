#include <Inventor/nodes/SoPerspectiveCamera.h>
#include "Camera.h"
 
Camera::Camera() 
{}

Camera::Camera(const SbVec3f& camera_position,
		const SbVec3f& camera_rotation_axis, 
		const float& camera_rotation_angle, 
		const float& camera_aspect_ratio,
		const SbVec3f& camera_direction, 
		const SbVec3f& camera_up, 
		const float& camera_height_angle)
{
	position = camera_position;
	rotation_axis = camera_rotation_axis;
	rotation_angle = camera_rotation_angle;
	aspect_ratio = camera_aspect_ratio;
	direction = camera_direction;
	view_up = camera_up;
	height_angle = camera_height_angle;
	
	setCoordSystem();
}

Camera::~Camera()
{}

void Camera::setCoordSystem() {
	calculateN();
	calculateU();
	calculateV();
}

void Camera::calculateN() {
	n = direction * (-1);
	n.normalize();
}

void Camera::calculateU() {
	u = view_up.cross(n);
	u.normalize();
}

void Camera::calculateV() {
	v = n.cross(u);  // already normalized
}
