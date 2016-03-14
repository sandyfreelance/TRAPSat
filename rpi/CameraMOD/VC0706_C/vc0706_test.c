/*
 * Test program for vc0706.cpp
 * compile with: $g++ vc0706_test.cpp -o vc0706_test -lwiringPi
 */
#include <string.h>
#include "vc0706_core.h"

int main (int argc, char* argv[])
{
	Camera cam;
	init(&cam);

	clock_t begin, end;
	int j=1;

	if(argv[1][0]-'0' > 0)
		j=argv[1][0]-'0';

	if(getVersion(&cam) != NULL)
	{
	  int i;
	  for (i=0; i<j; i++) 
	  {
		char *path;
		sprintf(path, "images/%d.jpg", i);
		takePicture(&cam, path); // stores to ./images/i.jpeg
	  }
	}

	return 0;
}
