/*
 * Test program for vc0706.cpp
 * compile with: $g++ vc0706_test.cpp -o vc0706_test -lwiringPi
 */
#include <string.h>
#include "vc0706_core.h"

int main (int argc, char* argv[])
{
	Camera cam;
	//printf("Camera declared.\n");

	init(&cam);
	//printf("Cam initialized.\n");

	if(getVersion(&cam) != 0)
	{
		takePicture(&cam, "images/test.jpg");
		printf("picture taken.\n");
	}
	else
	{
		printf("getVersion(&cam) == NULL");
	}

	return 0;
}
