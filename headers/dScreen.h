#ifndef _dSCREEN_
#define _dSCREEN_

#include <Screen.h>
#include <OS.h>
#include <Screen.h>
#include "dColor.h"
#include "dBitmap.h"
#include "dPalette.h"

class dScreen: public BWindow
{
public:
	dScreen(const char *title);
	virtual ~dScreen();
	bool Draw(dBitmap *buffer);
	void SetPalette(dPalette *p);

	void WaitVbl();

	virtual void Quit();
	virtual bool QuitRequested();
private:
	BScreen *screen; // the BScreen object, used for monitor beam sync
	BView *view; // the view that draws everything.
	BBitmap *offscreen_bitmap; // a bitmap

	dPalette *current_palette; // a pointer to the current palette

	rgb_color *screen_base;

	sem_id disconnected; // is locked when the screen is disconnected
	sem_id ready_to_quit; // is released when the screen can delete itself
	bool quitting; // is true when the screen wants to quit
};

inline void dScreen::SetPalette(dPalette *p)
{
	if (!quitting) current_palette = p;
}

inline void dScreen::WaitVbl()
{
	screen->WaitForRetrace();
}

#endif // _dSCREEN_
