

#include <iostream>
#include <math.h>
#include <vector>
#include <sstream>
#include <string>

#include <Inventor/SbLinear.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoPointLight.h>

#include "RayTracer.h"
#include "OSUInventor.h"

using namespace std;

RayTracer::RayTracer() {}

RayTracer::~RayTracer() {}

void RayTracer::read_scene(int argc, char **argv)
{
	time(&seconds);
	srand((unsigned int)seconds);
	focal_length = 8;
	dof_num_rays = 25;
	soft_shadow_num_rays = 25;
	
	// resolution
	stringstream x_str(argv[3]);
	stringstream y_str(argv[4]);
	x_str >> x;
	y_str >> y;
	if (!x_str || !y_str) {
		std::cerr << "<xres> and <yres> should be integers (ie. 500 500)" << std::endl;
		exit(10);
	}
	
	// filenames
	in_filename = argv[1];
	out_filename = argv[2];
	/*
	// shadow, refraction, depth of field
	stringstream a_str(argv[5]);
	stringstream b_str(argv[6]);
	stringstream c_str(argv[7]);
	a_str >> shadow_flag;
	b_str >> refraction_on;
	c_str >> depth_of_field_on;
	if (!a_str || !b_str || !c_str) {
		std::cerr << "<shadow_flag> should be integers 0, 1, or 2 and <refraction_on> <depth_of_field_on> should be integers 0 or 1" << std::endl;
		exit(10);
	}
	*/

	stringstream a_str(argv[5]);
	a_str >> finalscene;
	if(!a_str) {
		std::cerr << "<final_scene_on> should be integers 0 or 1" << std::endl;
		exit(10);
	}
	
	reflection_on = 1;
	shadow_flag = 1;
	refraction_on = 1;
	depth_of_field_on = 0;

	MAX_RECURSION = 3;

	SoDB::init();

	OSUInventorScene *scene = new OSUInventorScene(in_filename); 

	cout << "Read objects from file " << in_filename << "." << endl;
	cout << "Number of objects = " << scene->Objects.getLength() << "." << endl;

	// list objects
	for (int i = 0; i < scene->Objects.getLength(); i++) {
		OSUObjectData * obj = (OSUObjectData *)scene->Objects[i];

		if (!obj->Check()) {
			cerr << "Error detected in OSUObjectData for object " << i << "." << endl;
			exit(20);
		};

		SoType shape_type = obj->shape->getTypeId();

		cout << "Object " << i << " is a " << shape_type.getName().getString() << "." << endl;

		// object transformation
		SoTransform * transformation = obj->transformation;
		SbVec3f scale_vector = transformation->scaleFactor.getValue();
		SbRotation rotation = transformation->rotation.getValue();
		SbVec3f rotation_axis;
		float rotation_angle;
		rotation.getValue(rotation_axis, rotation_angle);
		SbVec3f translation_vector = transformation->translation.getValue();
		cout << "  Scale by (" << scale_vector[0] << "," << scale_vector[1] << "," << scale_vector[2] << ")." << endl;
		cout << "  Rotate around axis ("  << rotation_axis[0] << "," << rotation_axis[1] << "," << rotation_axis[2] << ")"  << " by " << rotation_angle << " radians." << endl;
		cout << "  Translate by ("  << translation_vector[0] << "," << translation_vector[1] << "," << translation_vector[2] << ")." << endl;

		// object material (color)
		SoMaterial * material = obj->material;

		Color ambient_color = Color(material->ambientColor[0][0], material->ambientColor[0][1], material->ambientColor[0][2]);
		cout << "  Ambient color (red, green, blue) = (" << ambient_color.red << "," << ambient_color.green << "," << ambient_color.blue << ")." << endl;

		// material->diffuseColor[0] is the first entry in an array of colors
		Color diffuse_color = Color(material->diffuseColor[0][0], material->diffuseColor[0][1], material->diffuseColor[0][2]);
		cout << "  Diffuse color (red, green, blue) = (" << diffuse_color.red << "," << diffuse_color.green << "," << diffuse_color.blue << ")." << endl;

		Color specular_color = Color(material->specularColor[0][0], material->specularColor[0][1], material->specularColor[0][2]);
		cout << "  Specular color (red, green, blue) = (" << specular_color.red << "," << specular_color.green << "," << specular_color.blue << ")." << endl;

		const float* s = material->shininess.getValues(0);
        float shininess = s[0];    //remember to assign shininess to object
        const float* t = material->transparency.getValues(0);
        float transparency = t[0];

		cout << "  Shininess = " << shininess << "." << endl;
		cout << "  Transparency = " << transparency << "." << endl;

		// set transformation matrix
		SbMatrix T,S, R, F; 
		T.setTranslate(translation_vector); 
		R.setRotate(rotation); 
		S.setScale(scale_vector); 
		//  F = T*R*S;    changed this to  F = S*R*T below  to be consistent with what used in ivview 
		F = S * R * T;	
		
		// multiply with original position
		SbVec3f pos(0,0,0); 
		F.multVecMatrix(pos, pos);

		if (shape_type == SoSphere::getClassTypeId()) {
			SoSphere * sphere = (SoSphere *) obj->shape;

			bool isSphere = false;
			float radius = sphere->radius.getValue();
			if (scale_vector[0] == scale_vector[1] == scale_vector[2]) {
				radius *= scale_vector[0];  // scale radius, assuming uniform
				isSphere = true;
			}
			cout << "  Sphere radius = " << radius << "." << endl;

			// add sphere
			Sphere s = Sphere(ambient_color, diffuse_color, specular_color, radius, pos, shininess, transparency, F);
			
			if (!isSphere)
				s.type = "ellipse";

			spheres.push_back(s);
		};

		if (shape_type == SoCube::getClassTypeId()) {
			SoCube * cube = (SoCube *) obj->shape;
			float width = cube->width.getValue();
			float height = cube->height.getValue();
			float depth = cube->depth.getValue();
			width *= scale_vector[0];
			height *= scale_vector[1];
			depth *= scale_vector[2];

			cout << "  Cube width, height, depth = (" << width << "," << height << "," << depth << ")." << endl;
			
			// add cube
			Cube c = Cube(ambient_color, diffuse_color, specular_color, width, height, depth, pos, shininess, transparency, F);
			cubes.push_back(c);
		};
	};

	// add cylinder
	
	// ambient_color,diffuse_color,specular_color,radius,height,center,shininess,transparency

	if (finalscene == 1) {
		Cylinder cyl = Cylinder(Color(0.2, 0.2, 0.2), Color(0.7, 0.9, 0.1), Color(1, 1, 1), 3, 1.5, SbVec3f(1, -1.7, -3), 0.6, 0);
		cylinders.push_back(cyl);
	}

	// set reference to spheres, cubes and cylinders
	for(vector<int>::size_type i = 0; i < spheres.size(); i++) {
		Object* s_ptr = &spheres[i];
		objects.push_back(s_ptr);
	}
	for(vector<int>::size_type i = 0; i < cubes.size(); i++) {
		Object* c_ptr = &cubes[i];
		objects.push_back(c_ptr);
	}
	if (finalscene == 1) {
		for (vector<int>::size_type i = 0; i < cylinders.size(); i++) {
			Object* cyl_ptr = &cylinders[i];
			objects.push_back(cyl_ptr);
		}
	}


	// list lights
	cout << endl;
	for (int j = 0; j < scene->Lights.getLength(); j++) {
		SoLight * light = (SoLight *) scene->Lights[j];
		SoType light_type = light->getTypeId();

		if (light->on.getValue() == true)
			cout << "Light " << j << " is on." << endl;
		else
			cout << "Light " << j << " is off." << endl;

		float intensity = light->intensity.getValue();
		SbVec3f color = light->color.getValue();

		cout << "  Light color = (" << color.getValue()[0] << ", " << color.getValue()[1] << ", " << color.getValue()[2]  << ")." << endl;
		cout << "  Light intensity = " << intensity << "." << endl;

		if (light_type == SoPointLight::getClassTypeId()) {
			SoPointLight * point_light = (SoPointLight *) light;
			SbVec3f location = point_light->location.getValue();
			cout << "  Point light location = ("  << location[0] << "," << location[1] << "," << location[2] << ")." << endl;

			// add point light
			lights.push_back(Light(light->on.getValue(), location, Color(color.getValue()[0], color.getValue()[1], color.getValue()[2]), intensity));
		};
	};

	// camera
	if (scene->Camera == NULL) {
		cout << endl;
		cout << "No camera found." << endl;
	}
	else {

		SoCamera * camera = scene->Camera;
		SbVec3f camera_position = camera->position.getValue();
		SbRotation camera_orientation = camera->orientation.getValue();
		SbVec3f camera_rotation_axis;
		float camera_rotation_angle;
		camera_orientation.getValue(camera_rotation_axis, camera_rotation_angle);
		float camera_aspect_ratio = camera->aspectRatio.getValue();
		SoType camera_type = camera->getTypeId();

		// calculate camera direction and camera up direction
		SbVec3f camera_direction, camera_up;
		camera_orientation.multVec(SbVec3f(0, 0, -1), camera_direction);
		camera_orientation.multVec(SbVec3f(0, 1, 0), camera_up);

		cout << endl;
		cout << "Camera position = (" << camera_position[0] << "," << camera_position[1] << "," << camera_position[2] << ")." << endl;
		cout << "Camera rotation axis = ("  << camera_rotation_axis[0] << "," << camera_rotation_axis[1] << "," << camera_rotation_axis[2] << ")." << endl;
		cout << "Camera rotation angle = " << camera_rotation_angle << " radians." << endl;
		cout << "Camera direction = ("  << camera_direction[0] << "," << camera_direction[1] << "," << camera_direction[2] << ")." << endl;
		cout << "Camera up direction = (" << camera_up[0] << "," << camera_up[1] << "," << camera_up[2] << ")." << endl;
		cout << "Camera width/height aspect ratio = " << camera_aspect_ratio << "." << endl;


		if (camera_type == SoPerspectiveCamera::getClassTypeId()) {
			// camera is a perspective camera
			SoPerspectiveCamera * perspective_camera = (SoPerspectiveCamera *) camera;
			float camera_height_angle = perspective_camera->heightAngle.getValue();
			cout << "Perspective camera height angle = " << camera_height_angle << " radians." << endl;

			// set camera object
			cam = Camera(camera_position, camera_rotation_axis, camera_rotation_angle, 
				camera_aspect_ratio, camera_direction, camera_up, camera_height_angle);
		};
	};
}

void RayTracer::trace_rays() {

	/**** Set Up Image Plane ****/
	float d = 0.1f; //r.cam.n.length();  // distance between image plane and camera
	float h = 2 * d * tan(cam.height_angle/2);  // height of image plane
	float w = h * cam.aspect_ratio;  // width of image plane
	SbVec3f c = cam.position - cam.n * d;  // center of image plane
	SbVec3f l = c - cam.u*(w/2) - cam.v*(h/2);  // left bottom corner of image plane

	// main raytrace loop
	for (int j = y-1; j >= 0; j--) {
		for (int i = 0; i < x; i++) {

			// calculate ray origin and direction
			SbVec3f s = l + cam.u * i * (w/x) + cam.v * j * (h/y);  // pixel center
			SbVec3f rayDirection = s - cam.position;  // s - e
			rayDirection.normalize();

			if (depth_of_field_on == 1) {
				// position of pixel on focal plane in rayDirection
				SbVec3f pointAimed = cam.position + focal_length * rayDirection;
					
				float r = 1;
				Color storecolors(0, 0, 0);
				for (int di = 0; di < dof_num_rays; di++) {
					float du = getRandomJitter();
					float dv = getRandomJitter();

					// creat new cam position using jittering
					SbVec3f start = cam.position - (r/2)*cam.u - (r/2)*cam.v + r*(du)*cam.u + r*dv*cam.v;
					SbVec3f direction = pointAimed - start;
					direction.normalize();
					Color temp = shade(start, direction, 1, i, j);
					storecolors += temp;
				}
				//average pixel colors
				Color pixelcolor((storecolors.red / dof_num_rays), (storecolors.green / dof_num_rays), (storecolors.blue / dof_num_rays));
				pixelcolor = threshold(pixelcolor);

				pixels.push_back(pixelcolor);
			
			}
			// don't do depth of field
			else {
				Color pixelcolor = shade(cam.position, rayDirection, 1, i, j);
				pixels.push_back(pixelcolor);
			}
		}
	}

	// write colors into image
	writeToImage();
	
	cout << "Done." << endl;

}

Color RayTracer::shade(const SbVec3f& startingRay, const SbVec3f& rayDirection, const int& recursionDepth, const int& mainx, const int& mainy) {
	Color c = Color(0, 0, 0);  // default black
	float t_min = 1000000;
	bool intersectFlag = false;
	float k = 0.2;  // use constant k
	float q = 20;  // glossiness

	Object* obj;
	float t;

	// loop through objects
	for (vector<int>::size_type i = 0; i < objects.size(); i++) {

		if (objects[i]->type == "ellipse") {
			SbMatrix m = objects[i]->transformation.inverse();
			SbVec3f temp1, temp2;
			m.multDirMatrix(startingRay, temp1);
			m.multDirMatrix(rayDirection, temp2);
			t = objects[i]->intersection(temp1, temp2);
			/*SbMatrix m = objects[i]->transformation.inverse();
			SbVec3f temp1, temp2;
			m.multVecMatrix(startingRay, temp1);
			m.multDirMatrix(rayDirection, temp2);
			t = objects[i]->intersection(temp1, temp2);*/
		}
		else {
			t = objects[i]->intersection(startingRay, rayDirection);
		}

		if (t < t_min) {
			obj = objects[i];
			t_min = t;
			intersectFlag = true;
		}
	}

	if (intersectFlag) {
		c = Color(0, 0, 0);

		SbVec3f p, normal;

		if (obj->type == "ellipse") {
			SbMatrix m = obj->transformation.inverse();
			SbVec3f temp1, temp2;
			m.multDirMatrix(startingRay, temp1);
			m.multDirMatrix(rayDirection, temp2);
			p = temp1 + t_min*temp2;
			normal = obj->getNormal(p);
			obj->transformation.multDirMatrix(p, p);
			m.transpose().multDirMatrix(normal, normal);
			/*SbMatrix m = obj->transformation.inverse();
			SbVec3f temp1, temp2;
			m.multVecMatrix(startingRay, temp1);
			m.multDirMatrix(rayDirection, temp2);
			p = temp1 + t_min*temp2;
			normal = obj->getNormal(p);
			obj->transformation.multVecMatrix(p, p);
			m.transpose().multDirMatrix(normal, normal);*/
		}
		else {
			p = cam.position + t_min*rayDirection;
			normal = obj->getNormal(p);
		}

		if (obj->type == "cube")
			obj->diffuse_color = checkers(mainx, mainy, p[0], p[1]);

		if (obj->type == "cylinder")
			obj->diffuse_color = grain(mainx, mainy, p[0], p[1]);


		// loop through light sources
		for (vector<int>::size_type i = 0; i < lights.size(); i++) {

			SbVec3f l = lights[i].position - p;  // light vector
			l.normalize();
			SbVec3f r = 2 * (normal.dot(l)) * normal - l;
			SbVec3f v = startingRay - p;
			r.normalize();
			v.normalize();
			float d_factor = normal.dot(l);
			float s_factor = pow(v.dot(r), q);

			// ambient
			c += Color(k * lights[i].intensity * lights[i].color.red * obj->ambient_color.red,
					k * lights[i].intensity * lights[i].color.green * obj->ambient_color.green, 
					k * lights[i].intensity * lights[i].color.blue * obj->ambient_color.blue);

			if (obj->transparency > 0)
				c = c * obj->transparency;
			
			//SbVec3f p2 = p + .01f*l;  // add some epsilon

			// check for shadow
			if (shadow_flag == 1)  { // hard shadow
				bool inShadow = false;
				SbVec3f p2 = p + .01f*l; 
				inShadow = intersectShadowRay(p2, l, normal);

				if (!inShadow) {
					// check if light is in front of object 
					if (normal.dot(l) >= 0) {
	
						// diffuse
						c += Color(lights.at(i).intensity * lights[i].color.red * max<float>(d_factor, 0.0) * obj->diffuse_color.red,
							lights[i].intensity * lights[i].color.green * max<float>(d_factor, 0.0) * obj->diffuse_color.green,
							lights[i].intensity * lights[i].color.blue * max<float>(d_factor, 0.0) * obj->diffuse_color.blue);			
							
						// specular
						c += Color(lights[i].intensity * lights[i].color.red * max<float>(s_factor, 0.0) * obj->specular_color.red,
							lights[i].intensity * lights[i].color.green * max<float>(s_factor, 0.0) * obj->specular_color.green,
							lights[i].intensity * lights[i].color.blue * max<float>(s_factor, 0.0) * obj->specular_color.blue);

						if (obj->transparency > 0) {
							c = c * obj->transparency;			
							c = c * obj->transparency;
						}
						
					}
				}
			}

			else if (shadow_flag == 2) { // soft shadow
				float value = 0.0f;
				bool inShadow = false;
				SbVec3f nn = -l;
				SbVec3f v_up(0, 0, 0);
				float epsilon = 0.01;
				SbVec3f p2 = p + .01f*l;
				
				// set v_up
				if(l[0] >= l[1] && l[0] >= l[2])
					v_up.setValue(1,0,0);
				if(l[1] >= l[0] && l[1] >= l[2])
					v_up.setValue(0,1,0);
				if(l[2] >= l[0] && l[2] >= l[1])
					v_up.setValue(0,0,1);

				SbVec3f uu = v_up.cross(nn);
				SbVec3f vv = nn.cross(uu);
				
				for (int di = 0; di < soft_shadow_num_rays; di++) {
					float du = getRandomJitter();
					float dv = getRandomJitter();
					SbVec3f shadowRayDirection = (lights[i].position + uu*cos(3.0*du)*3.0 + vv*sin(3.0*dv)*3.0) - p2;
					shadowRayDirection.normalize();
					SbVec3f shadowRayStart = p2 + epsilon*shadowRayDirection;

					if (normal.dot(shadowRayDirection) < 0) {
						inShadow = true;
					}
					else {
						inShadow = intersectShadowRay(p2, l, normal);
					}

					if (!inShadow) {
						
						// add a factor of 1/25 of the color since exposed to light
						if (normal.dot(l) >= 0) {
							
							// diffuse
							c += Color(lights.at(i).intensity * lights[i].color.red * max<float>(d_factor, 0.0) * obj->diffuse_color.red,
								lights[i].intensity * lights[i].color.green * max<float>(d_factor, 0.0) * obj->diffuse_color.green,
								lights[i].intensity * lights[i].color.blue * max<float>(d_factor, 0.0) * obj->diffuse_color.blue)*(1/soft_shadow_num_rays);			
								
							// specular
							c += Color(lights[i].intensity * lights[i].color.red * max<float>(s_factor, 0.0) * obj->specular_color.red,
								lights[i].intensity * lights[i].color.green * max<float>(s_factor, 0.0) * obj->specular_color.green,
								lights[i].intensity * lights[i].color.blue * max<float>(s_factor, 0.0) * obj->specular_color.blue)*(1/soft_shadow_num_rays);
						}
					}
						
				}
			}

				
			if (recursionDepth < MAX_RECURSION) {

				if (reflection_on == 1) {
					// if object is shiny
					if (obj->shininess > 0) {
						SbVec3f v2 = rayDirection;//= p - startingRay;
						v2.normalize();
						SbVec3f reflection = -2 * (v2.dot(normal)) * normal + v2;
						reflection.normalize();
						SbVec3f p2 = p + .01f*reflection;

						if(obj->transparency > 0)
							c += Color(shade(p2, reflection, recursionDepth + 1, mainx, mainy) * (obj->shininess*0.0025) * (1 - obj->transparency));
						else
							c += Color(shade(p2, reflection, recursionDepth + 1, mainx, mainy) * (obj->shininess*0.0025));
					}
				}

				if (refraction_on == 1) {
					// if object is transparent
					if (obj->transparency > 0) {
						SbVec3f v2 = rayDirection;//p - startingRay;
						v2.normalize();
						

						// check if ray is going into the medium
						if (normal.dot(v2) < 0) {  // going into the sphere
							float nr = 1.0f / 1.5f;
							float root = sqrtf(1 - nr * nr * (1 - normal.dot(-v2) * normal.dot(-v2)));
							if (root >= 0.0) {
								SbVec3f refraction = (nr * (normal.dot(-v2)) - root)*normal - (nr*-v2);
								refraction.normalize();
								SbVec3f p2 = p + .01f*refraction;
								c += Color(shade(p2, refraction, recursionDepth + 1, mainx, mainy) * (obj->transparency*0.01f));
							}
						}
						else {  //going out of the object
							float nr = 1.5f / 1.0f;
							float root = sqrtf(1 - nr * nr * (1 - (-normal.dot(-v2)) * (-normal.dot(-v2))));
							if (root >= 0.0) {
								SbVec3f refraction = (nr * (-normal.dot(-v2)) - root)*-normal - (nr*-v2);
								refraction.normalize();
								SbVec3f p2 = p + .01f*refraction;
								c += Color(shade(p2, refraction, recursionDepth + 1, mainx, mainy) * (obj->transparency*0.01f));
							}
						}
					}
				} // end refraction
			} // end recursion
			
		} // end of lights

	}  // end of if(intersection)

	c = threshold(c*255);

	return Color(c.red, c.green, c.blue);
}

Color RayTracer::threshold(const Color& c) {
	Color temp = c;
	if (temp.red > 255)
		temp.red = 255;
	if (temp.green > 255)
		temp.green = 255;
	if (temp.blue > 255)
		temp.blue = 255;
	/*if (temp.red < 15)
		temp.red = 15;
	if (temp.green < 15)
		temp.green = 15;
	if (temp.blue < 15)
		temp.blue = 15;*/
	return temp;
}

bool RayTracer::intersectShadowRay(const SbVec3f& p, const SbVec3f& l, const SbVec3f normal) {

	bool inShadow = false;

	// loop through objects
	for (vector<int>::size_type i = 0; i < objects.size(); i++) {
		if (objects[i]->transparency == 0) {  // if transparent, no shadow
			float t = objects[i]->shadowIntersection(p, l);
			if (t == 1) {
				inShadow = true;
				break;
			}
		}
	}

	return inShadow;
}

void RayTracer::writeToImage() {
	 FILE *fp; 
     fp = fopen(out_filename,"w");  // file name is out.ppm
     int height = y; //  assume ImageHeight is defined and initialized 
     int width = x;  //   same for width 
     int max_ccv = 255; 
     fprintf(fp,"P3 %d %d %d\n", width, height, max_ccv);  // write the header 

	 int count = -1;

	 for (int j = height-1; j >= 0; j--) {
		for (int i = 0; i < width; i++) {
			count++;
			fprintf(fp, "%f %f %f ", pixels[count].red, pixels[count].green, pixels[count].blue);
        } 
		fprintf(fp, "\n");
	}
     fclose(fp); 
}

float RayTracer::getRandomJitter() {
	return rand()/(float)(RAND_MAX+1); 
}

Color RayTracer::checkers(const float& mainx, const float& mainy, const float& sx, const float& sy) {
	int jump = ((int)(50+sx/0.2) + (int)(50+sy/0.2)) % 2;
	if (jump == 0)
		return Color(0, 0, 0);
	else
		return Color(1, 1, 1);
}

Color RayTracer::grain(const float& mainx, const float& mainy, const float& sx, const float& sy) {
	float r = sqrtf(sx*sx + sy*sy);
	float temp = r / 0.15 + 0.1*sin(0.2/.2);
	float jump = ((int)temp) % 2;
	if (jump == 0)
		return Color(0, 0, 0);
	else
		return Color(1, 1, 1);
}
