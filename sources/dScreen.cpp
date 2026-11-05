#include <Application.h>
#include <View.h>
#include <Window.h>
#include <Bitmap.h>
#include <stdio.h>

#include "dScreen.h"

dScreen::dScreen(const char *title):
		BWindow(BRect(10,10,10 + 639, 10 + 399),title,B_TITLED_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	disconnected=create_sem(1,"drawing_sem");
	ready_to_quit=create_sem(0,"sync_sem");

	quitting = false;
	screen = new BScreen(this);
	
	view = new BView(BRect(0,0,639,399),"view",B_FOLLOW_NONE,B_WILL_DRAW);
	AddChild(view);
	offscreen_bitmap = new BBitmap(BRect(0,0,639,399),B_RGB32);
	screen_base = (rgb_color*)offscreen_bitmap->Bits();
};	

dScreen::~dScreen()
{
	ASSERT(this != 0);

	delete offscreen_bitmap;
	delete screen;
	printf("screen deleted\n");
	delete_sem(disconnected);
}

void dScreen::Quit()
{
//	printf("Quit() quitting = true\n");
	quitting = true;
//	printf("trying to acquire ready_to_quit sem\n");
	acquire_sem(ready_to_quit);
//	printf("Quit() acquired ready_to_quit sem\n");
	BWindow::Quit();
}

bool dScreen::QuitRequested()
{
//	printf("QuitRequested");
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

bool dScreen::Draw(dBitmap *bitmap)
{
//	printf("Draw()\n");

//	printf("quitting = %s\n",quitting?"true":"false");
	if (quitting)
	{
//		printf("Draw() quitting, releasing ready_to_quit sem\n");
		release_sem(ready_to_quit);
		return false;
	}

//	printf("Draw() trying to acquireing sem disconnected\n");
	while( acquire_sem_etc(disconnected,1,B_RELATIVE_TIMEOUT,10000) != B_NO_ERROR)
	{
//		printf("checking if we should quit..\n");
		if (quitting)
		{
//			printf("Draw() quitting, releasing ready_to_quit sem\n");
			release_sem(ready_to_quit);
			return false;
		}
	}
//	printf("Draw() disconnected sem acquired\n");

	rgb_color *base = screen_base;
	rgb_color *stop = screen_base + 640 * 400;
	
	uint8 *buffer = (uint8*)bitmap->Bits();
	
	while (base < stop)
	{
		base[0] = current_palette->palette[buffer[0]];
		base[1] = current_palette->palette[buffer[1]];
		base[2] = current_palette->palette[buffer[2]];
		base[3] = current_palette->palette[buffer[3]];
		base[4] = current_palette->palette[buffer[4]];
		base[5] = current_palette->palette[buffer[5]];
		base[6] = current_palette->palette[buffer[6]];
		base[7] = current_palette->palette[buffer[7]];

		base+=8;
		buffer+=8;
	}

	if (Lock())
	{
		view->DrawBitmap(offscreen_bitmap,BPoint(0,0));
		Unlock();
	}
	release_sem(disconnected);
//	printf("Draw() releasing sem disconnected\n");
	return true;
}

