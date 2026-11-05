#ifndef _dBITMAP_
#define _dBITMAP_

#include "dColor.h"
#include "dFace.h"
#include "dObject.h"
#include "dClut2.h"

#include <Debug.h>
#include <Directory.h>
#include <stdlib.h>

class dBitmap
{
public:
	dBitmap(int32 width,int32 height,int32 num_polys = 0);
	dBitmap(int32 width,int32 height,BDirectory *dir,const char *filename,int32 num_polys = 0);
	virtual ~dBitmap();

	int32 Width();
	int32 Height();

	dColor *Bits();

	void Empty();

	void swap(int32 &val1,int32 &val2);

	void Read(BFile *file);

// –––––––––––––––––––––––––––––––– 2D stuff

	void PutPixel(uint32 x,uint32 y,dColor color);
	void PutSubPixel(dVertex *dot,dClut2 *clut); 				// OBS uint32 is 16:16 fixed point
	void AddSubPixel(dVertex *dot);
	void DrawSubPixelPoints(dObject *obj,dClut2 *clut); 		// the object should be rotated & projected

	void AddLine(dVertex *start,dVertex *end);
	void LineClut(dVertex *start,dVertex *end,dClut2 *clut);

	void DrawBitmap(int32 x,int32 y,dBitmap *bitmap);
	void DrawBitmapAdd(int32 x,int32 y,dBitmap *bitmap);
	void DrawBitmap(int32 x,int32 y,dBitmap *bitmap,dBitmap *stencil_bitmap,uint8 value);
	void DrawBitmap(int32 x,int32 y,dBitmap *bitmap,dBitmap *stencil_bitmap,uint8 value,dColor border);

// –––––––––––––––––––––––––––––––– 3D stuff

	void DrawFaceTextured(dFace *face,dBitmap *texture);
	void HorizLineTextured(dColor *line_pointer,int32 linestart,int32 lineend,dColor *texture,int32 u,int32 v,int32 add_u,int32 add_v);

	void DrawFaceTexturedClut(dFace *face,dBitmap *texture,dClut2 *clut);
	void HorizLineTexturedClut(dColor *line_pointer,int32 linestart,int32 lineend,dColor *texture,int32 u,int32 v,int32 add_u,int32 add_v,dClut2 *clut);

	void DrawFaceFlat(dFace *face,dColor color);
	void HorizLineFlat(dColor *line_pointer,int32 linestart,int32 lineend,dColor color);

	void DrawFaceFlatClut2(dFace *face,dColor color,dClut2 *clut);
	void HorizLineFlatClut2(dColor *line_pointer,int32 linestart,int32 lineend,dColor color,dClut2 *clut);

	void AddObject(dObject *obj,bool backface_culling = true);
	void Render();	

	int32 max_u,max_v;
	int32 min_u,min_v;

private:
// the clip-region
	int16 clp_left,clp_top,clp_right,clp_bottom;

// used for sorting the faces
	int sort_count0,sort_count1,sort_count2,sort_count3;
	dPolygon **sort_list0;
	dPolygon **sort_list1;
	dPolygon **sort_list2;
	dPolygon **sort_list3;

	int32 list_size; // the number of polygons the sort-list can contain

// the actual bitmap
	dColor *buffer;
// the width and the height of the bitmap
	int32 width,height;
};

inline int32 dBitmap::Width()
{
	ASSERT(this != NULL);

	return width;
}

inline int32 dBitmap::Height()
{
	ASSERT(this != NULL);

	return height;
}

inline dColor *dBitmap::Bits()
{
	ASSERT(this != NULL);

	return buffer;
}

inline void dBitmap::PutPixel(uint32 x,uint32 y,dColor color)
{
	ASSERT(this != NULL);
	ASSERT(x < Width());
	ASSERT(y < Height());
	
	buffer[y * Width() + x] = color;
}

inline void dBitmap::PutSubPixel(dVertex *point,dClut2 *clut)
{
	int32 x = point->x * 65535.0;
	int32 y = point->y * 65535.0;

	if ((x>>16) < 0) return;
	if ((y>>16) < 0) return;

	if ((x>>16) > Width()-2)	return;
	if ((y>>16) > Height()-2)	return;
	
	dColor *p00 = buffer + ( (y>>16) * Width() ) + (x>>16);
	dColor *p01 = p00 + 1;
	dColor *p10 = p00 + Width();
	dColor *p11 = p10 + 1;
	
	uint32 x1 = (x >> 8) & 255;
	uint32 x0 = 255 - x1;
	uint32 y1 = (y >> 8) & 255;
	uint32 y0 = 255 - y1;

	*p00 = p00->Blend(clut,dColor((x0 * y0) >> 8));
	*p01 = p01->Blend(clut,dColor((x1 * y0) >> 8));
	*p10 = p10->Blend(clut,dColor((x0 * y1) >> 8));
	*p11 = p11->Blend(clut,dColor((x1 * y1) >> 8));
}

inline void dBitmap::AddSubPixel(dVertex *point)
{
	uint32 x = point->x * 65535.0;
	uint32 y = point->y * 65535.0;

	if ((x>>16) > Width()-2)	return;
	if ((y>>16) > Height()-2)	return;

	dColor *p00 = buffer + ( (y>>16) * Width() ) + (x>>16);
	dColor *p01 = p00 + 1;
	dColor *p10 = p00 + Width();
	dColor *p11 = p10 + 1;
	
	if ( ((x >> 8) & 63) & ((y >> 8) & 63) == 0)
	{
		*p00 = dColor(255);
		*p01 = dColor(255);
		p01[1] = dColor(255);
		p01[640] = dColor(255);
		p01[641] = dColor(255);

		*p10 = dColor(255);
		*p11 = dColor(255);
		p11[1] = dColor(255);
		p11[640] = dColor(255);
		p11[641] = dColor(255);

		return;
	}
	
	uint32 x1 = (x >> 8) & 255;
	uint32 x0 = 255 - x1;
	uint32 y1 = (y >> 8) & 255;
	uint32 y0 = 255 - y1;

	*p00 = *p00 + dColor((x0 * y0) >> 8);
	*p01 = *p01 + dColor((x1 * y0) >> 8);
	*p10 = *p10 + dColor((x0 * y1) >> 8);
	*p11 = *p11 + dColor((x1 * y1) >> 8);

}

inline void dBitmap::DrawSubPixelPoints(dObject *obj,dClut2 *clut)
{
	for (int32 c = 0 ; c < obj->VertexCount() ; c++ )
		PutSubPixel(obj->ProjectedVertex(c),clut);
}

inline void dBitmap::Empty()
{
	uint32 *frame = (uint32*)buffer;
	uint32 *frame_stop = (uint32*)(buffer + Width() * Height());

	while(frame<frame_stop)
	{
		frame[0] = 0;
		frame[1] = 0;
		frame[2] = 0;
		frame[3] = 0;
		frame[4] = 0;
		frame[5] = 0;
		frame[6] = 0;
		frame[7] = 0;
		frame += 8;
	}
}

inline void dBitmap::swap(int32 &val1,int32 &val2)
{
	register int32 tmp=val1;
	val1=val2;
	val2=tmp;
}

/*
0									linestart								lineend
|										|											|
V										V											V
..............................*********************************........................
line_pointer
*/

inline void dBitmap::HorizLineTextured(dColor *line_pointer,int32 linestart,int32 lineend,dColor *texture,int32 u,int32 v,int32 add_u,int32 add_v)
{
	if (linestart < clp_left)
	{
		u += (clp_left - linestart) * add_u;
		v += (clp_left - linestart) * add_v;
		linestart = clp_left;
	}
	if (lineend > clp_right+1) lineend = clp_right+1;

	dColor *run_pointer = line_pointer + linestart;
	dColor *stop_pointer = line_pointer + lineend;

	while (run_pointer < stop_pointer)
	{
		*run_pointer = texture[(((v>>16)&511)<<9) + ((u>>16)&511)];
		run_pointer++;
		u += add_u;
		v += add_v;
	}
}

inline void dBitmap::HorizLineTexturedClut(dColor *line_pointer,int32 linestart,int32 lineend,dColor *texture,int32 u,int32 v,int32 add_u,int32 add_v,dClut2 *clut)
{
	if (linestart < clp_left)
	{
		u += (clp_left - linestart) * add_u;
		v += (clp_left - linestart) * add_v;
		linestart = clp_left;
	}
	if (lineend > clp_right+1) lineend = clp_right+1;

	dColor *run_pointer = line_pointer + linestart;
	dColor *stop_pointer = line_pointer + lineend;

	while (run_pointer < stop_pointer)
	{
		*run_pointer = run_pointer->Blend(clut,texture[(((v>>16)&511)<<9) + ((u>>16)&511)]);
		run_pointer++;
		u += add_u;
		v += add_v;
	}
}

inline void dBitmap::HorizLineFlat(dColor *line_pointer,int32 linestart,int32 lineend,dColor color)
{
	lineend++;
	if (linestart < clp_left) linestart = clp_left;
	if (lineend > clp_right+1) lineend = clp_right+1;
//	if (lineend > clp_right) lineend = clp_right;

//	fprintf(stderr,"start: %ld end: %ld\n",linestart,lineend);
	dColor *run_pointer = line_pointer + linestart;
	dColor *stop_pointer = line_pointer + lineend;

	int lim3 = stop_pointer - run_pointer - 1;
	
	if (lim3 > 0)
		while (lim3--)
		{
			*run_pointer = color;
			run_pointer++;
		}
}

inline void dBitmap::HorizLineFlatClut2(dColor *line_pointer,int32 linestart,int32 lineend,dColor color,dClut2 *clut)
{
	lineend++;
	if (linestart < clp_left) linestart = clp_left;
	if (lineend > clp_right+1) lineend = clp_right+1;
//	if (lineend > clp_right) lineend = clp_right;

//	fprintf(stderr,"start: %ld end: %ld\n",linestart,lineend);
	dColor *run_pointer = line_pointer + linestart;
	dColor *stop_pointer = line_pointer + lineend;

	int lim = stop_pointer - run_pointer - 1;
	
	if (lim > 0)
		while (lim--)
		{
			*run_pointer = run_pointer->Blend(clut,color);
			run_pointer++;
		}
}

#endif // _dBITMAP_
