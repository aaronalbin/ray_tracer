#include <iostream>
#include "RayTracer.h"

void usage_error();

int main(int argc, char *argv[])
{	
	if (argc != 6)
		usage_error();

	RayTracer r;
	r.read_scene(argc, argv);
	r.trace_rays();

	return 0;
}

void usage_error()
{
	std::cerr << "Usage: lab_04 <input.iv> <output.ppm> <xres> <yres> <final_scene_on>" << std::endl;
	exit(10);
}