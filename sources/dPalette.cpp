#include "dPalette.h"

#include <math.h>
#include <stdio.h>
#include <File.h>

dPalette::dPalette()
{
	for (int32 c = 0;c < 256 ; c++)
	{
		rgb_color color = {	(uint8)(pow(c/255.0,.4)*255),
								 (uint8)(pow(c/255.0,.4)*255),
								 (uint8)(pow(c/255.0,.4)*255),
								 0};
		Set(c,color);
	}
}

dPalette::dPalette(BDirectory *dir,const char *filename)
{
	BFile file(dir,filename,B_READ_ONLY);
	if (file.InitCheck() != B_OK)
	{
		printf("Error loading palette \"%s\"\n",filename);
		return;
	}
	
	for (int32 c = 0;c < 256 ; c++)
	{
		uint8 rgb[3];
		file.Read((void*)rgb,3);
		rgb_color color = {
								(uint8)(pow(rgb[2]/255.0,.8)*255),
								(uint8)(pow(rgb[1]/255.0,.8)*255),
								(uint8)(pow(rgb[0]/255.0,.8)*255),
								0
								};
		Set(c,color);
	}
}
