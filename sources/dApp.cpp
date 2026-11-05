#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <Alert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <StorageDefs.h>

#include <Roster.h>

#include "dScreen.h"
#include "dContourObject.h"
#include "dPart3DScene.h"
#include "dPart3DScene2.h"
#include "dPart3DSceneSpin2.h"
#include "dPart3DSceneDots.h"
#include "dPart3DSceneTwist.h"
#include "dApp.h"

#define DEBUG

int32 dApp::main_thread(void *parameter)
{
	dScreen *screen = ((dApp::parameter*)parameter)->screen;
	dApp *app = ((dApp::parameter*)parameter)->app;

	app_info theInfo;
	be_app->GetAppInfo(&theInfo);
	BEntry tEnt(&theInfo.ref);
	BDirectory app_root;
	tEnt.GetParent(&app_root);

	BPath app_path;
	tEnt.GetPath(&app_path);
	app_path.GetParent(&app_path);

	// an out of tree build keeps the binary in build/, with data/ one level up
	BEntry data_entry;
	if (app_root.FindEntry("data", &data_entry) != B_OK)
	{
		BPath up(app_path);
		if (up.GetParent(&up) == B_OK)
		{
			BDirectory up_dir(up.Path());
			if (up_dir.FindEntry("data", &data_entry) == B_OK)
			{
				app_root.SetTo(up.Path());
				app_path = up;
			}
		}
	}

// –––––––––––––––––––––––––––––––––

// music
	char tmp[B_PATH_NAME_LENGTH];
	tmp[0] = 0;
	strcat(tmp,app_path.Path());
	strcat(tmp,"/data/Spruce-ComicFakery.NNN");
	app->Load(tmp);

// frames
	dBitmap *frame1 = app->frame1 = new dBitmap(640,400,5000);
	dBitmap *frame2 = app->frame2 = new dBitmap(640,400,5000);

// loading palette
	dPalette *red_palette = app->red_palette = new dPalette(&app_root,"data/pal-red.raw");
	dPalette *green_palette = app->green_palette = new dPalette(&app_root,"data/pal-green.raw");
	dPalette *blue_palette = app->blue_palette = new dPalette(&app_root,"data/pal-blue.raw");
	dPalette *yellow_palette = app->yellow_palette = new dPalette(&app_root,"data/pal-yellow.raw");

// loading textures
	dBitmap *tex_generic = app->tex_generic = new dBitmap(512,512,&app_root,"data/tex-generic.raw");
	dBitmap *tex_credits = app->tex_credits = new dBitmap(512,512,&app_root,"data/tex-credits.raw");
	dBitmap *tex_greets = app->tex_greets = new dBitmap(512,512,&app_root,"data/tex-greetz.raw");
	dBitmap *tex_linefeed = app->tex_linefeed = new dBitmap(512,512,&app_root,"data/tex-linefeed.raw");
	dBitmap *envmap = app->envmap = new dBitmap(512,512,&app_root,"data/env-3lights.raw");

// backgrounds
	dBitmap *flum = app->flum = new dBitmap(640,400,&app_root,"data/bg-flum.raw");
	dBitmap *eyes = app->eyes = new dBitmap(640,400,&app_root,"data/bg-eyes1.raw");
	dBitmap *eyes2 = app->eyes2 = new dBitmap(640,400,&app_root,"data/bg-eyes2.raw");
	dBitmap *logo = app->logo = new dBitmap(640,400,&app_root,"data/bg-linefeed.raw");

// wipes
	dBitmap *wipe_hole = app->wipe_hole = new dBitmap(640,400,&app_root,"data/wipe-hole.raw");
	dBitmap *wipe_ripple = app->wipe_ripple = new dBitmap(640,400,&app_root,"data/wipe-ripple.raw");
	dBitmap *wipe_twirl = app->wipe_twirl = new dBitmap(640,400,&app_root,"data/wipe-twirl.raw");
	dBitmap *wipe_square = app->wipe_square = new dBitmap(640,400,&app_root,"data/wipe-square.raw");

// load cluts
	dClut2 *clut_mix50 = app->clut_mix50 = new dClut2(&app_root,"data/clut-mix50.raw");
	dClut2 *clut_mix70 = app->clut_mix70 = new dClut2(&app_root,"data/clut-mix70.raw");
	dClut2 *clut_mix90 = app->clut_mix90 = new dClut2(&app_root,"data/clut-mix90.raw");
	dClut2 *clut_screen60 = app->clut_screen60 = new dClut2(&app_root,"data/clut-screen60.raw");
	dClut2 *clut_diff100 = app->clut_diff100 = new dClut2(&app_root,"data/clut-diff100.raw");

	dClut2 *iclut_invmul = app->iclut_invmul = new dClut2(&app_root,"data/iclut-invmul.raw");
	dClut2 *iclut_lighten = app->iclut_lighten = new dClut2(&app_root,"data/iclut-lighten.raw");

// load objects

	dObject *poles_big = app->poles_big = new dObject(0.0,0.0,5.0);
	poles_big->Load(&app_root,"data/obj-polesbig.lwo");
	poles_big->materials->type = MATERIAL_CLUT_TEXTURED;
	poles_big->materials->color = dColor(195);
	poles_big->materials->clut = clut_screen60;
	poles_big->materials->texture = tex_linefeed;
	poles_big->WrapTexture();

	dContourObject *poles_big_c = app->poles_big_c = new dContourObject(0.0,0.0,5.0);
	poles_big_c->Load(&app_root,"data/obj-polesbig.lwo");
	poles_big_c->materials->type = MATERIAL_CLUT;
	poles_big_c->materials->clut = iclut_invmul;

	dObject *cubes = app->cubes = new dObject(0.0,0.0,5.0);
	cubes->Load(&app_root,"data/obj-cubes.lwo");
	cubes->materials->type = MATERIAL_CLUT_TEXTURED;
	cubes->materials->color = dColor(192);
	cubes->materials->clut = clut_mix50;
	cubes->materials->texture = tex_generic;
	cubes->WrapTexture();

	dContourObject *cubes_c = app->cubes_c = new dContourObject(0.0,0.0,5.0);
	cubes_c->Load(&app_root,"data/obj-cubes.lwo");
	cubes_c->materials->type = MATERIAL_CLUT;
	cubes_c->materials->clut = iclut_invmul;

	dObject *poles = app->poles = new dObject(0.0,0.0,5.0);
	poles->Load(&app_root,"data/obj-poles.lwo");
	poles->materials->type = MATERIAL_CLUT_TEXTURED;
	poles->materials->color = dColor(195);
	poles->materials->clut = clut_mix50;
	poles->materials->texture = tex_credits;
	poles->WrapTexture();

	dContourObject *poles_c = app->poles_c = new dContourObject(0.0,0.0,5.0);
	poles_c->Load(&app_root,"data/obj-poles.lwo");
	poles_c->materials->type = MATERIAL_CLUT_TEXTURED;
	poles_c->materials->color = dColor(195);
	poles_c->materials->clut = clut_mix50;
	poles_c->materials->texture = tex_generic;
	poles_c->WrapTexture();

	dObject *ball_cred = app->ball_cred = new dObject(0.0,0.0,5.0);
	ball_cred->Load(&app_root,"data/obj-ball.lwo");
	ball_cred->materials->type = MATERIAL_CLUT_TEXTURED;
	ball_cred->materials->color = dColor(195);
	ball_cred->materials->clut = clut_mix50;
	ball_cred->materials->texture = tex_credits;
	ball_cred->WrapTexture();

	dObject *ball_greet = ball_cred->Clone();
	ball_cred->MorphScale(1.6,ball_greet);
	ball_greet->materials->texture = tex_greets;
	ball_greet->materials->clut = clut_screen60;

	dObject *cigarrs = app->cigarrs = new dObject(0.0,0.0,5.0);
	cigarrs->Load(&app_root,"data/obj-cigarrs.lwo");
	cigarrs->materials->type = MATERIAL_CLUT_ENVMAPPED;
	cigarrs->materials->texture = envmap;
	cigarrs->materials->clut = clut_mix90;
	cigarrs->WrapTexture();

	dObject *cigarrs_c = app->cigarrs_c = new dObject(0.0,0.0,5.0);
	cigarrs_c->LoadLines(&app_root,"data/obj-cigarrs.lwo");
	cigarrs_c->materials->type = MATERIAL_CLUT;
	cigarrs_c->materials->clut = iclut_invmul;

	dObject *nut = app->nut = new dObject(0.0,0.0,5.0);
	nut->Load(&app_root,"data/obj-nut.lwo");
	nut->materials->type = MATERIAL_CLUT_ENVMAPPED;
	nut->materials->texture = envmap;
	nut->materials->clut = clut_mix50;

	dContourObject *nut_c = app->nut_c = new dContourObject(0.0,0.0,5.0);
	nut_c->Load(&app_root,"data/obj-nut.lwo");
	nut_c->materials->type = MATERIAL_CLUT;
	nut_c->materials->clut = iclut_lighten;

	dObject *nut_w = app->nut_w = new dObject(0.0,0.0,5.0);
	nut_w->LoadLines(&app_root,"data/obj-nut.lwo");
	nut_w->materials->type = MATERIAL_CLUT;
	nut_w->materials->clut = iclut_lighten;

	dObject *linefeed_5 = app->linefeed_5 = new dObject(0.0,0.0,5.0);
	linefeed_5->Load(&app_root,"data/obj-5.lwo");
	linefeed_5->materials->type = MATERIAL_CLUT_FLAT;
	linefeed_5->materials->color = dColor(8);
	linefeed_5->materials->clut = clut_mix50;

	dObject *linefeed_5_w = app->linefeed_5_w = new dObject(0.0,0.0,5.0);
	linefeed_5_w->LoadLines(&app_root,"data/obj-5.lwo");
	linefeed_5_w->materials->type = MATERIAL_CLUT;
	linefeed_5_w->materials->clut = iclut_invmul;

// ––––––––––––––––––––––––––––––––

	dPart3DScene2 *part1 = app->part1 = new dPart3DScene2(poles_big,poles_big_c,eyes2,3);
	dPart3DScene2 *part2 = app->part2 = new dPart3DScene2(cubes,cubes_c,eyes2,5);
	dPart3DScene2 *nut_scene = app->nut_scene = new dPart3DScene2(nut,nut_c,flum,1.5);

	dPart3DSceneDots *cut_scene = app->cut_scene = new dPart3DSceneDots(cubes,logo,iclut_lighten,2);
	dPart3DSceneTwist *cigarr_scene = app->cigarr_scene = new dPart3DSceneTwist(cigarrs,cigarrs_c,eyes,1.8);
	dPart3DSceneSpin2 *cred_scene = app->cred_scene = new dPart3DSceneSpin2(ball_cred,ball_greet,eyes2,3);

	dPart3DSceneDist2 *cubes_scene = app->cubes_scene = new dPart3DSceneDist2(cubes,cubes_c,flum,4);
	dPart3DSceneDist2 *poles_scene = app->poles_scene = new dPart3DSceneDist2(poles,poles_c,eyes,4);

	dPart3DSceneTwist *linefeed_scene = app->linefeed_scene = new dPart3DSceneTwist(linefeed_5,linefeed_5_w,eyes,2);
	dPart3DSceneTwist *cubes_twist_scene = app->cubes_twist_scene = new dPart3DSceneTwist(cubes,cubes_c,eyes2,4);

	uint32 *list = new uint32[640*400]; 
	BFile list_file(&app_root,"data/table-zoom2.raw",B_READ_ONLY);
	if (list_file.InitCheck() == B_OK)
		list_file.Read(list,640*400*4);
	else
		memset(list,0,640*400*4);

// –––––––––––––––––––––––––––––––––

	screen->SetPalette(blue_palette);

// START!
	uint16 frames = 0;

	bigtime_t start_time = system_time();
	float time = 0.0;
	float part_start;

	bool resume = true;

	do
	{
		switch (app->CurrentPart())
		{
		case 0:// PART ************* LOGO
			app->SetPos(0);
			app->Start();
//			printf("start\n");
			part_start = time;
			do
			{
				// fade in
				rgb_color col = {0,0,0,0};
				dPalette tmp = blue_palette->Mix(col,1.0 - (time-part_start)/5.0);
				cut_scene->Render(frame1,time);
				//screen->WaitVbl();
				snooze(20000);
				screen->SetPalette(&tmp);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 5.0);
			if (!resume) return 0;

			part_start = time;
			do
			{
				cut_scene->Render(frame1,time);
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 0);
			if (!resume) return 0;
		break;
		case 1:// PART ************* WIPE TO POLES SCENE WITH CLUT FLAT
			poles_big->materials->clut = clut_screen60;
			part_start = time;
			do
			{
				// wipe
				part1->Render(frame2,time);

				frame1->DrawBitmap(0,0,frame2,wipe_twirl,(time - part_start)/3.0 * 255.0);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 3.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				part1->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 1);
			if (!resume) return 0;
		break;
		case 2:// PART ************* FADE TO GREEN PALETTE POLES SCENE WITH CLUT TEXTURE
			part_start = time;
			do
			{
				part1->Render(frame1,time);	

				dPalette tmp = blue_palette->Mix(green_palette,(time-part_start)/3.0);
				screen->SetPalette(&tmp);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 3.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				part1->Render(frame1,time);	

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 2);
			if (!resume) return 0;
		break;
		case 3:// PART ************* WIPE TO POLES SCENE WITH NEW CLUT
			poles_big->materials->clut = clut_diff100;
			part_start = time;
			do
			{
				// wipe
				part1->Render(frame2,time);
				frame1->DrawBitmap(0,0,frame2,wipe_square,(time - part_start)/2.0 * 255.0);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 2.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				part1->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 3);
			if (!resume) return 0;
		break;
		case 4:// PART ************* WIPE TO CUT SCENE & CHANGE PALETTE TO YELLOW
			part_start = time;
			do
			{
				// wipe
				cut_scene->Render(frame2,time);
				frame1->DrawBitmap(0,0,frame2,wipe_twirl,(time - part_start)/6.0 * 255.0);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 6.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				dPalette tmp = green_palette->Mix(yellow_palette,(time-part_start)/2.0);
				cut_scene->Render(frame1,time);
				screen->SetPalette(&tmp);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 2.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				cut_scene->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 4);
			if (!resume) return 0;
		break;
		case 5:// PART ************* Wipe to nut scene
			part_start = time;
			do
			{
				nut_scene->Render(frame2,time);
				frame1->DrawBitmap(0,0,frame2,wipe_ripple,(time - part_start)/2.0 * 255.0);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 2.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				nut_scene->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 5);
			if (!resume) return 0;
		break;
		case 6:// PART ************* WIPE TO CIGARR SCENE
			part_start = time;
			do
			{
				cigarr_scene->Render(frame2,time/5);
				frame1->DrawBitmap(0,0,frame2,wipe_hole,(time-part_start)/3.0 * 255.0);
				dPalette tmp = yellow_palette->Mix(red_palette,(time-part_start)/6.0);
				screen->SetPalette(&tmp);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 3.0);
			part_start = time;
			do
			{
				cigarr_scene->Render(frame1,time/5);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 6);
			if (!resume) return 0;
		break;
		case 7:// PART ************* ZOOM & PART3
			cubes->materials->clut = clut_screen60;
			cubes_c->materials->clut = iclut_invmul;
			part_start = time;
			switch (rand()&3)
			{
				case 0:
				screen->SetPalette(yellow_palette);
				break;
				case 1:
				screen->SetPalette(red_palette);
				break;
				case 2:
				screen->SetPalette(green_palette);
				break;
				case 3:
				screen->SetPalette(blue_palette);
				break;
			}
			cubes_scene->Render(frame1,time);
			do
			{
				cubes_scene->amount = (1.0 - (time - part_start) / 3.0) * .3;
				if (cubes_scene->amount < 0.0) cubes_scene->amount = 0.0;

				cubes_scene->Render(frame2,time);

				for(int32 i=0;i<640*400;i++)
					frame2->Bits()[i] = frame2->Bits()[i].Blend(clut_mix70,frame1->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
				if (!screen->Draw(frame2)) return 0;

				cubes_scene->amount = (1.0 - (time - part_start) / 3.0) * .3;
				if (cubes_scene->amount < 0.0) cubes_scene->amount = 0.0;

				cubes_scene->Render(frame1,time);

				for(int32 i=0;i<640*400;i++)
					frame1->Bits()[i] = frame1->Bits()[i].Blend(clut_mix70,frame2->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 7);

			if (!resume) return 0;
		break;
		case 8:// PART ************* ZOOM & PART3
			poles->materials->clut = clut_screen60;
			poles_c->materials->clut = iclut_invmul;
			part_start = time;
			switch (rand()&3)
			{
				case 0:
				screen->SetPalette(yellow_palette);
				break;
				case 1:
				screen->SetPalette(red_palette);
				break;
				case 2:
				screen->SetPalette(green_palette);
				break;
				case 3:
				screen->SetPalette(blue_palette);
				break;
			}
			poles_scene->Render(frame1,time);
			do
			{
				poles_scene->amount = (1.0 - (time - part_start) / 3.0) * .3;
				if (poles_scene->amount < 0.0) poles_scene->amount = 0.0;

				poles_scene->Render(frame2,time);

				for(int32 i=0;i<640*400;i++)
					frame2->Bits()[i] = frame2->Bits()[i].Blend(clut_mix70,frame1->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
				if (!screen->Draw(frame2)) return 0;

				poles_scene->amount = (1.0 - (time - part_start) / 3.0) * .3;
				if (poles_scene->amount < 0.0) poles_scene->amount = 0.0;

				poles_scene->Render(frame1,time);

				for(int32 i=0;i<640*400;i++)
					frame1->Bits()[i] = frame1->Bits()[i].Blend(clut_mix70,frame2->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 8);

			if (!resume) return 0;
		break;
		case 9:// PART ************* CRED_PART
			part_start = time;
			screen->SetPalette(blue_palette);
			do
			{
				cred_scene->Render(frame1,time);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 9);
			if (!resume) return 0;
		break;
		case 10:// PART ************* TWIST LINEFEED 5
			linefeed_scene->Render(frame1,time);
			do
			{
				linefeed_scene->Render(frame2,time);

				for(int32 i=0;i<640*400;i++)
					frame2->Bits()[i] = frame2->Bits()[i].Blend(clut_mix70,frame1->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
				if (!screen->Draw(frame2)) return 0;

				linefeed_scene->Render(frame1,time);

				for(int32 i=0;i<640*400;i++)
					frame1->Bits()[i] = frame1->Bits()[i].Blend(clut_mix70,frame2->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 10);
			if (!resume) return 0;
		break;
		case 11:// PART ************* TWIST LINEFEED 5
			linefeed_scene->Render(frame1,time);
			do
			{
				linefeed_scene->Render(frame2,time + 1);

				for(int32 i=0;i<640*400;i++)
					frame2->Bits()[i] = frame2->Bits()[i].Blend(clut_mix70,frame1->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
				if (!screen->Draw(frame2)) return 0;

				linefeed_scene->Render(frame1,time + 1);

				for(int32 i=0;i<640*400;i++)
					frame1->Bits()[i] = frame1->Bits()[i].Blend(clut_mix70,frame2->Bits()[list[i]]);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 11);
			if (!resume) return 0;
		break;
		case 12:// PART ************* NUT_WIREFRAME
			screen->SetPalette(yellow_palette);
			part_start = time;
			nut->materials->clut = clut_mix50;
			nut->materials->type = MATERIAL_CLUT_ENVMAPPED;
			nut->materials->texture = envmap;
			nut_w->materials->clut = iclut_lighten;
			nut_w->materials->type = MATERIAL_CLUT;
			do
			{
				nut->MoveTo(sin(time/3)*.1,sin(time/4)*.1,1);
				nut->RotateTo(time/3,time/4,time/2);
				nut->Project();

				nut_w->MoveTo(sin(time/3)*.1,sin(time/4)*.1,1);
				nut_w->RotateTo(time/3,time/4,time/2);
				nut_w->Project();

				frame1->DrawBitmap(0,0,flum);
				frame1->AddObject(nut);
				frame1->AddObject(nut_w);
				frame1->Render();

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 12);
			if (!resume) return 0;
		break;
		case 13:// PART ************* NUT_CONTOUR
			part_start = time;
			nut_c->materials->clut = iclut_lighten;
			nut->materials->clut = clut_mix50;
			nut->materials->type = MATERIAL_CLUT_ENVMAPPED;
			nut->materials->texture = envmap;
			do
			{
				nut_c->MoveTo(sin(time/3)*.1,sin(time/4)*.1,1);
				nut_c->RotateTo(time/3,time/4,time/2);
				nut_c->Project();

				nut->MoveTo(sin(time/3)*.1,sin(time/4)*.1,1);
				nut->RotateTo(time/3,time/4,time/2);
				nut->Project();

				frame1->DrawBitmap(0,0,flum);
				frame1->AddObject(nut);
				frame1->AddObject(nut_c);
				frame1->Render();
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 13);
			if (!resume) return 0;
		break;
		case 14:// PART ************* NUT_CONTOUR
			part_start = time;
			do
			{
				nut_scene->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 14);
			if (!resume) return 0;
		break;
		case 15:// PART ************* NUT_CONTOUR
			do
			{
				part1->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 15);
			if (!resume) return 0;
		break;
		case 16:// PART ************* NUT_CONTOUR
			do
			{
				part1->Render(frame1,time);
		
				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 16);
			if (!resume) return 0;
		break;
		case 17:// PART ************* NUT_CONTOUR
			part_start = time;
			do
			{
				// fade out
				rgb_color col = {255,255,255,0};
				dPalette tmp = blue_palette->Mix(col,1.0 - (time-part_start)/3.0);
				cut_scene->Render(frame1,time);
				//screen->WaitVbl();
				snooze(20000);
				screen->SetPalette(&tmp);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 3.0);
			if (!resume) return 0;
			part_start = time;
			do
			{
				// fade out
				rgb_color col = {0,0,0,0};
				dPalette tmp = blue_palette->Mix(col,(time-part_start)/7.0);
				cut_scene->Render(frame1,time);
				//screen->WaitVbl();
				snooze(20000);
				screen->SetPalette(&tmp);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && time < part_start + 7.0);
			if (!resume) return 0;
			do
			{
				snooze(10000);
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 17);
			if (!resume) return 0;
			app->Stop();
//			printf("stop\n");
		break;
		case 18:
			screen->SetPalette(red_palette);
			part_start = time;
			cubes->materials->type = MATERIAL_CLUT_FLAT;
			cubes->materials->clut = clut_mix50;
			cubes->materials->color = dColor(192);
			cubes_c->materials->clut = iclut_lighten;
			do
			{
				cubes_twist_scene->Render(frame1,time/4);

				time = ((float)(system_time() - start_time))/1000000.0;
				frames++;
			} while ( (resume = screen->Draw(frame1)) && app->CurrentPart() == 18);
			if (!resume) return 0;
		break;
		case 19:
			app->current_part = 0;
			break;
		}

		} while (true);

	be_app->PostMessage(B_QUIT_REQUESTED);
	while (screen->Draw(frame1)); // the screen class has to know that we know it wants to quit

	return 0;
}

void dApp::Break(uint32 cmd)
{
	if (cmd < 20) current_part = cmd;
}


dApp::dApp():BApplication("application/x-vnd.Linefeed-demo"),stsp()
{
	dPalette *p = new dPalette();
/*
	for (int c = 0;c < 256 ; c++)
	{
		rgb_color bosse = {	(uint8)(sqrt(c/255.0)*255),
									(uint8)(sqrt(c/255.0)*255),
									(uint8)(sqrt(c/255.0)*255),
									0};
		p->Set(c, bosse );
	}
*/
	for (int c = 0;c < 256 ; c++)
	{
		rgb_color bosse = {	(uint8)0,
									(uint8)0,
									(uint8)0,
									0};
		p->Set(c, bosse );
	}


	screen = new dScreen("\\n - released at Midwinter 00, by linefeed");
	
	//if ( error != B_OK ) PostMessage(B_QUIT_REQUESTED);
	screen->SetPalette(p);
	screen->Show();

	//parameter param;
	param.screen = screen;
	
	param.app = this;
	thread = spawn_thread(main_thread,"main_thread",B_NORMAL_PRIORITY,(void*)&param);
	resume_thread(thread);
	
	current_part = 0;
}

dApp::~dApp()
{
	delete part1;
	delete part2;
	delete cubes_scene;
	
	delete poles_big;
	delete poles_big_c;
	delete cubes;
	delete cubes_c;
	delete linefeed_5;

	delete clut_mix50;
	delete clut_screen60;
	delete clut_diff100;
	delete iclut_invmul;
	delete iclut_lighten;

	delete tex_generic;
	delete tex_linefeed;
	delete tex_credits;
	delete tex_greets;

	delete envmap;
	delete eyes;
	delete logo;

	delete wipe_ripple;
	delete wipe_hole;
	delete wipe_twirl;

	delete red_palette;
	delete green_palette;
	delete blue_palette;

	delete frame1;
	delete frame2;

	BAlert *infoAlert = new BAlert("secret thread","Released by linefeed at Midwinter 00,\numeå, sweden\n\ncode, code & code:\nhydri (arvid@iname.com)\nsfx, gfx & code:\nspruce (jn@earthling.net)\n\nSawteeth softsynth\nhttp://www.acc.umu.se/~bedev/\nwere used for the groovy sound\n","Wow",NULL,NULL,B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_IDEA_ALERT);
	infoAlert->SetShortcut(0,B_ESCAPE);
	infoAlert->SetShortcut(0,B_OK);
	infoAlert->Go();
}

int main()
{
	dApp *app;
	app = new dApp(); 
	printf("app run start\n");
	app->Run();
	printf("app run end\n");
	delete app;
	return 0;	
};
