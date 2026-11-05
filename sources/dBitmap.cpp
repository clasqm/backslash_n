#include "dBitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <Directory.h>
#include <File.h>

dBitmap::dBitmap(int32 w,int32 h,BDirectory *dir,const char *filename,int32 num_polys)
{
	width = w;
	height = h;
	list_size = num_polys;

	if (width & 3) fprintf(stderr,"WARNING!, bitmap width %d, have to be even 8\n",width);
	if (height & 3) fprintf(stderr,"WARNING!, bitmap height %d, have to be even 8\n",height);

	clp_left=0;
	clp_right=width;
	clp_top=0;
	clp_bottom=height;

// initialize the sort buffers
	sort_count0 = 0;
	sort_list0 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN
	sort_list1 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN
	sort_list2 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN
	sort_list3 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN

	buffer = new dColor[Width() * Height()];

	BFile file(dir,filename,B_READ_ONLY);
	if (file.InitCheck() != B_NO_ERROR)
	{
		printf("Couldn't open file \"%s\"\n",filename);
		return;
	}
	file.Read(buffer,Width()*Height());

	min_u = 150;
	min_v = 150;
	max_u = 150;
	max_v = 150;
}

dBitmap::dBitmap(int32 w,int32 h,int32 num_polys)
{
	width = w;
	height = h;
	list_size = num_polys;
	
	if (width & 3) fprintf(stderr,"WARNING!, bitmap width %d, have to be even 8\n",width);
	if (height & 3) fprintf(stderr,"WARNING!, bitmap height %d, have to be even 8\n",height);

	clp_left=0;
	clp_right=width-1;
	clp_top=0;
	clp_bottom=height-1;

// initialize the sort buffers
	sort_count0 = 0;
	sort_list0 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN
	sort_list1 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN
	sort_list2 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN
	sort_list3 = new dPolygon*[list_size];	// BÖR ALLOKERAS NÄR OBJEKT LADDATS IN

	buffer = new dColor[Width() * Height()];

	min_u = 150;
	min_v = 150;
	max_u = 150;
	max_v = 150;
}

dBitmap::~dBitmap()
{
	delete []buffer;
	
	delete []sort_list0;
	delete []sort_list1;
	delete []sort_list2;
	delete []sort_list3;
}

void dBitmap::Render()
{
//	printf("Render()\n");

	dPolygon **point0;
	dPolygon **point1;
	dPolygon **point2;
	dPolygon **point3;
	dPolygon **stop;
	
	sort_count1=0;

	int testbit=1;
	for (int c = 31 ; c != 0 ; c-- ){

		// list0
		sort_count2 = 0;
		sort_count3 = 0;
		point0=sort_list0;
		point1=sort_list1;
		point2=sort_list2;
		point3=sort_list3;

		stop = point0+sort_count0;
		for (;point0 < stop ; point0++)
		{
			float z = (*point0)->Z();
			if ( (*((int*) &(z)) & testbit) !=0)
			{
				*point3 = *point0;
				point3++;
				sort_count3++;
			}else{
				*point2 = *point0;
				point2++;
				sort_count2++;
			}
		}

		// list1
		stop = point1+sort_count1;
		for (;point1 < stop ; point1++)
		{
			float z = (*point1)->Z();
			if ( (*((int*)&(z)) & testbit) !=0)
			{
				*point3 = *point1;
				point3++;
				sort_count3++;
			}else{
				*point2 = *point1;
				point2++;
				sort_count2++;
			}
		}

		testbit<<=1;

		// list2
		sort_count0 = 0;
		sort_count1 = 0;
		point0=sort_list0;
		point1=sort_list1;
		point2=sort_list2;
		point3=sort_list3;

		stop = point2+sort_count2;
		for (;point2 < stop ; point2++)
		{
			float z = (*point2)->Z();
			if ( (*((int*)&(z)) & testbit) !=0){
				*point1 = *point2;
				point1++;
				sort_count1++;
			}else{
				*point0 = *point2;
				point0++;
				sort_count0++;
			}
		}

		// list3
		stop = point3+sort_count3;
		for (;point3 < stop ; point3++)
		{
			float z = (*point3)->Z();
			if (( *((int*)&(z)) & testbit) !=0){
				*point1 = *point3;
				point1++;
				sort_count1++;
			}else{
				*point0 = *point3;
				point0++;
				sort_count0++;
			}
		}
		
		testbit<<=1;
	}
	
	for (int i=sort_count1-1;i>=0;i--)
	{
		dPolygon *polygon=sort_list1[i];
		polygon->Draw(this);
	}

	for (int i=sort_count0-1;i>=0;i--)
	{
		dPolygon *polygon=sort_list0[i];
		polygon->Draw(this);
	}

	sort_count0 = 0;
	sort_count1 = 0;
	sort_count2 = 0;
	sort_count3 = 0;
}

void dBitmap::AddObject(dObject *obj,bool backface_culling)
{
	if (sort_count0 == list_size) return;
	for (int i=0;i<obj->PolygonCount();i++)
	{
		if (!backface_culling || obj->Polygon(i)->IsFront())
		{
			dPolygon *polygon=obj->Polygon(i);
			if (polygon->material->type & MATERIAL_ENVMAPPED)
			{
				dVertex normal;
				normal=*obj->RotatedNormal(polygon->vex[0] - obj->ProjectedVertex(0));
				polygon->u[0]=(255.5+normal.x*255.0)*65500.0;
				polygon->v[0]=(255.5+normal.y*255.0)*65500.0;

				if (polygon->VertexCount() > 1)
				{
					normal=*obj->RotatedNormal(polygon->vex[1] - obj->ProjectedVertex(0));
					polygon->u[1]=(255.5+normal.x*255.0)*65500.0;
					polygon->v[1]=(255.5+normal.y*255.0)*65500.0;

					if (polygon->VertexCount() > 2)
					{
						normal=*obj->RotatedNormal(polygon->vex[2] - obj->ProjectedVertex(0));
						polygon->u[2]=(255.5+normal.x*255.0)*65500.0;
						polygon->v[2]=(255.5+normal.y*255.0)*65500.0;
					}
				}
			}
			polygon->CalcZ();
//			printf("adding poly to sort list\n");
			sort_list0[sort_count0] = polygon;
			sort_count0++;
			if (sort_count0 == list_size) return;
		}
	}
//	printf("num_polys_added %d\n",sort_count0);
}

void dBitmap::DrawBitmap(int32 x,int32 y,dBitmap *bitmap)
{
	register dColor pixel;
	// calculate how much will be clipped on each side
	uint16	margin_left=(x<clp_left)?clp_left-x:0,
				margin_top=(y<clp_top)?clp_top-y:0,
				margin_right=(x+bitmap->Width()-1>clp_right)?x+bitmap->Width()-clp_right-1:0,
				margin_bottom=(y+bitmap->Height()-1>clp_bottom)?y+bitmap->Height()-clp_bottom-1:0;

	if (x>clp_right || x+bitmap->Width()-1<clp_left || y>clp_bottom || y+bitmap->Height()-1<clp_top) return;

	// calculate the drawn width and height, after clipping that is
	uint16 dwidth=bitmap->Width()-margin_left-margin_right-1;
	uint16 dheight=bitmap->Height()-margin_top-margin_bottom;

	uint16 s_linefeed=Width()-dwidth-1;
	uint16 b_linefeed=bitmap->Width()-dwidth-1;

	// calculate the buffer offset for the the top left pixel
	dColor *buf=bitmap->Bits() + (margin_top)*bitmap->Width()+(margin_left);

	dColor *frame=buffer + (y+margin_top)*Width()+(x+margin_left);
	for(y=0;y<dheight;y++)
	{
		for(x=0;x<dwidth + 1;x++)
		{
			*frame=*buf;

			frame++;
			buf++;
		}
		frame+=s_linefeed;
		buf+=b_linefeed;
	}

}

void dBitmap::DrawBitmapAdd(int32 x,int32 y,dBitmap *bitmap)
{
	register dColor pixel;
	// calculate how much will be clipped on each side
	uint16	margin_left=(x<clp_left)?clp_left-x:0,
				margin_top=(y<clp_top)?clp_top-y:0,
				margin_right=(x+bitmap->Width()-1>clp_right)?x+bitmap->Width()-clp_right-1:0,
				margin_bottom=(y+bitmap->Height()-1>clp_bottom)?y+bitmap->Height()-clp_bottom-1:0;

	if (x>clp_right || x+bitmap->Width()-1<clp_left || y>clp_bottom || y+bitmap->Height()-1<clp_top) return;

	// calculate the drawn width and height, after clipping that is
	uint16 dwidth=bitmap->Width()-margin_left-margin_right-1;
	uint16 dheight=bitmap->Height()-margin_top-margin_bottom;

	uint16 s_linefeed=Width()-dwidth-1;
	uint16 b_linefeed=bitmap->Width()-dwidth-1;

	// calculate the buffer offset for the the top left pixel
	dColor *buf=bitmap->Bits() + (margin_top)*bitmap->Width()+(margin_left);

	dColor *frame=buffer + (y+margin_top)*Width()+(x+margin_left);
	for(y=0;y<dheight;y++)
	{
		for(x=0;x<dwidth + 1;x++)
		{
			*frame+=*buf;

			frame++;
			buf++;
		}
		frame+=s_linefeed;
		buf+=b_linefeed;
	}

}

void dBitmap::DrawBitmap(int32 x,int32 y,dBitmap *bitmap,dBitmap *stencil_bitmap,uint8 value)
{
	if (stencil_bitmap->Width() == bitmap->Width() && stencil_bitmap->Height() == bitmap->Height())

	register dColor pixel;
	// calculate how much will be clipped on each side
	uint16	margin_left=(x<clp_left)?clp_left-x:0,
				margin_top=(y<clp_top)?clp_top-y:0,
				margin_right=(x+bitmap->Width()-1>clp_right)?x+bitmap->Width()-clp_right-1:0,
				margin_bottom=(y+bitmap->Height()-1>clp_bottom)?y+bitmap->Height()-clp_bottom-1:0;

	if (x>clp_right || x+bitmap->Width()-1<clp_left || y>clp_bottom || y+bitmap->Height()-1<clp_top) return;

	// calculate the drawn width and height, after clipping that is
	uint16 dwidth=bitmap->Width()-margin_left-margin_right-1;
	uint16 dheight=bitmap->Height()-margin_top-margin_bottom;

	uint16 s_linefeed=Width()-dwidth-1;
	uint16 b_linefeed=bitmap->Width()-dwidth-1;

	// calculate the buffer offset for the the top left pixel
	dColor *buf=bitmap->Bits() + (margin_top)*bitmap->Width()+(margin_left);
	dColor *stencil=stencil_bitmap->Bits() + (margin_top)*bitmap->Width()+(margin_left);
	dColor *frame=buffer + (y+margin_top)*Width()+(x+margin_left);
	for(y=0;y<dheight;y++)
	{
		for(x=0;x<dwidth + 1;x++)
		{
			if ( *((uint8*)stencil) < value ) *frame=*buf;

			frame++;
			stencil++;
			buf++;
		}
		frame+=s_linefeed;
		buf+=b_linefeed;
		stencil+=b_linefeed;
	}

}

void dBitmap::DrawBitmap(int32 x,int32 y,dBitmap *bitmap,dBitmap *stencil_bitmap,uint8 value,dColor border)
{
	if (stencil_bitmap->Width() == bitmap->Width() && stencil_bitmap->Height() == bitmap->Height())

	register dColor pixel;
	// calculate how much will be clipped on each side
	uint16	margin_left=(x<clp_left)?clp_left-x:0,
				margin_top=(y<clp_top)?clp_top-y:0,
				margin_right=(x+bitmap->Width()-1>clp_right)?x+bitmap->Width()-clp_right-1:0,
				margin_bottom=(y+bitmap->Height()-1>clp_bottom)?y+bitmap->Height()-clp_bottom-1:0;

	if (x>clp_right || x+bitmap->Width()-1<clp_left || y>clp_bottom || y+bitmap->Height()-1<clp_top) return;

	// calculate the drawn width and height, after clipping that is
	uint16 dwidth=bitmap->Width()-margin_left-margin_right-1;
	uint16 dheight=bitmap->Height()-margin_top-margin_bottom;

	uint16 s_linefeed=Width()-dwidth-1;
	uint16 b_linefeed=bitmap->Width()-dwidth-1;

	// calculate the buffer offset for the the top left pixel
	dColor *buf=bitmap->Bits() + (margin_top)*bitmap->Width()+(margin_left);
	dColor *stencil=stencil_bitmap->Bits() + (margin_top)*bitmap->Width()+(margin_left);
	dColor *frame=buffer + (y+margin_top)*Width()+(x+margin_left);
	for(y=0;y<dheight;y++)
	{
		for(x=0;x<dwidth + 1;x++)
		{
			uint8 stencil_val = *((uint8*)stencil);
			if ( stencil_val < value ) *frame=*buf;
			else if ( stencil_val == value ) *frame = border;
			frame++;
			stencil++;
			buf++;
		}
		frame+=s_linefeed;
		buf+=b_linefeed;
		stencil+=b_linefeed;
	}

}

void dBitmap::LineClut(dVertex *start,dVertex *end,dClut2 *clut)
{
//	fprintf(stderr,"LINE: frame_buffer: %x\n",buffer);

	int32 x1=start->x*65536.0;
	int32 y1=start->y*65536.0;
	int32 x2=end->x*65536.0;
	int32 y2=end->y*65536.0;

	int32 clip_left=clp_left<<16;
	int32 clip_right=(clp_right)<<16;
	int32 clip_top=clp_top<<16;
	int32 clip_bottom=(clp_bottom)<<16;

// ******* CLIPPING START *************
	if( (x1<clip_left+0x10000 && x2<clip_left+0x10000) || (x1>clip_right && x2>clip_right) || (y1<clip_top+0x10000 && y2<clip_top+0x10000) || (y1>clip_bottom && y2>clip_bottom) ){
		return;
	}

	float dxy,dyx,dzx,dzy;
	dxy=(float)(x2-x1)/(float)(y2-y1);
	dyx=(float)(y2-y1)/(float)(x2-x1);
	if(x1<x2)
	{
		if(x1<clip_left+0x10000)
		{
			y1+=(int)(dyx*(clip_left-x1));
			x1=clip_left+1;
		}
		if(x2>(clip_right))
		{
			y2+=(int)(dyx*(clip_right-x2));
			x2=clip_right;
		}
	}
	else
	{
		if(x2<clip_left+0x10000)
		{
			y2+=(int)(dyx*(clip_left-x2));
			x2=clip_left+0x10000;
		}
		if(x1>(clip_right))
		{
			y1+=(int)(dyx*(clip_right-x1));
			x1=clip_right;
		}
	}
	if(y1<y2)
	{
		if(y1<clip_top+0x10000)
		{
			x1+=(int)(dxy*(clip_top-y1));
			y1=clip_top+0x10000;
		}
		if(y2>(clip_bottom))
		{
			x2+=(int)(dxy*(clip_bottom-y2));
			y2=clip_bottom;
		}
	}
	else
	{
		if(y2<clip_top+1)
		{
			x2+=(int)(dxy*(clip_top-y2));
			y2=clip_top+0x10000;
		}
		if(y1>(clip_bottom))
		{
			x1+=(int)(dxy*(clip_bottom-y1));
			y1=clip_bottom;
		}
	}



//	}


	if (x1<clip_left+0x10000 || y1<clip_top+0x10000 || x1>clip_right || y1>clip_bottom) return;

// ******** CLIPPING END **************

// ************ HARD CODED FOR 640 WIDE SCREENS **************

	int add;
	int c, lim;
	int of;
	int tmp1,tmp2;

	if (abs(x2-x1) > abs(y2-y1))
	{
		// BIG X - pretty optimized (hardcoded 640 width)
		if (x1>x2)
		{
			swap(x1,x2);
			swap(y1,y2);
		}


		if ((x2 - x1)>>16 != 0)
		{
			add = (y2-y1)/((x2-x1)>>16);
		
			x1 >>= 16;
			x2 >>= 16;
			lim = 1+x2-x1;
			while (lim != 0)
			{
				c=(y1 & 0xffff) >> 8;
	
				tmp1 = y1 >> 16;
				of=(tmp1<<9)+(tmp1<<7)+x1;
	
				buffer[of] = buffer[of].Blend(clut,dColor(255-c));
				buffer[of+640] = buffer[of+640].Blend(clut,dColor(c));
	
				y1+=add;
				x1++;
				lim--;
			}
		}
	}
	else
	{
		// BIG Y - this part is pretty optimized
		if (y1>y2)
		{
			swap(x1,x2);
			swap(y1,y2);
		}

		if ((y2 - y1)>>16 != 0)
		{
			add = (x2-x1)/((y2-y1)>>16);
			
			y1>>=16;
			y2>>=16;
			lim = 1+y2-y1;
			y1*=640;
			while (lim != 0)
			{
				c=(x1 & 0xffff) >> 8;
				of=y1+(x1>>16);

				buffer[of] = buffer[of].Blend(clut,dColor(255-c));
				buffer[of+1] = buffer[of+1].Blend(clut,dColor(c));

				x1+=add;
				y1+=640;
				lim--;
			}
		}
	}
}








void dBitmap::DrawFaceTextured(dFace *face,dBitmap *texture)
{
	if (face->vex[0]->z<0 || face->vex[1]->z<0 || face->vex[2]->z<0) return;
/*

	uint16 clp_left=clp_left+ScreenLeft(),clp_right=clp_right+ScreenLeft();
	uint16 clp_top=clp_top+ScreenTop(),clp_bottom=clp_bottom+ScreenTop();
*/

	int32 vx[3];
	int32 vy[3];
	int32 u[3];
	int32 v[3];
	
	vx[0]=face->vex[0]->x*65536;
	vy[0]=face->vex[0]->y;
	u[0]=face->u[0];
	v[0]=face->v[0];

	vx[1]=face->vex[1]->x*65536;
	vy[1]=face->vex[1]->y;
	u[1]=face->u[1];
	v[1]=face->v[1];

	vx[2]=face->vex[2]->x*65536;
	vy[2]=face->vex[2]->y;
	u[2]=face->u[2];
	v[2]=face->v[2];

	//sort by y hardco(re)ded bub-sort
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); swap(u[0],u[1]); swap(v[0],v[1]);}
	if (vy[1] > vy[2]) {	swap(vy[1],vy[2]); swap(vx[1],vx[2]); swap(u[1],u[2]); swap(v[1],v[2]);}
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); swap(u[0],u[1]); swap(v[0],v[1]);}

	if (vy[0] == vy[2]) return;

	int32 linestart,lineend;
	int32 add_start,add_end;
	int32 line_u_start,line_v_start;
	int32 u_line_add,v_line_add;
	int32 add_u,add_v;

	dColor *line_pointer = buffer + vy[0]	* Width();
	line_u_start = u[0];
	line_v_start = v[0];

	int32 middle_x=vx[0]+((vx[2]-vx[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));
	int32 middle_u=u[0]+((u[2]-u[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));
	int32 middle_v=v[0]+((v[2]-v[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));

	if (vx[1]>>16 == middle_x>>16) return;

	if (middle_x<vx[1])
	{
		add_u=(u[1]-middle_u)/(((vx[1]>>16)-(middle_x>>16)));
		add_v=(v[1]-middle_v)/(((vx[1]>>16)-(middle_x>>16)));
	}
	else
	{
		add_u=(middle_u-u[1])/(((middle_x>>16)-(vx[1]>>16)));
		add_v=(middle_v-v[1])/(((middle_x>>16)-(vx[1]>>16)));
	}

// ***** UPPER TRIANGLE *************
	
	if (vy[1] > vy[0])
	{
		linestart=lineend=vx[0];

		dColor *line_stop_pointer = buffer + vy[1] * Width();
	
		if (middle_x < vx[1])
		{
			add_start=(vx[2]-linestart)/(vy[2]-vy[0]);
			add_end=(vx[1]-lineend)/(vy[1]-vy[0]);

			u_line_add=(u[2]-u[0])/(vy[2]-vy[0]);
			v_line_add=(v[2]-v[0])/(vy[2]-vy[0]);
		}
		else
		{
			add_start=(vx[1]-linestart)/(vy[1]-vy[0]);
			add_end=(vx[2]-lineend)/(vy[2]-vy[0]);

			u_line_add=(u[1]-u[0])/(vy[1]-vy[0]);
			v_line_add=(v[1]-v[0])/(vy[1]-vy[0]);
		}
	
	// CLIP Y

		if (vy[0] < clp_top)
		{
			line_pointer += (clp_top - vy[0]) * Width();
			linestart	 += (clp_top - vy[0]) * add_start;
			lineend		 += (clp_top - vy[0]) * add_end;

			line_u_start += u_line_add * (clp_top-vy[0]);
			line_v_start += v_line_add * (clp_top-vy[0]);
		}

		if (vy[1]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineTextured(line_pointer,linestart>>16,lineend>>16,texture->Bits(),line_u_start,line_v_start,add_u,add_v);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
			
			line_u_start += u_line_add;
			line_v_start += v_line_add;
		}
	}

// ***** BOTTOM TRIANGLE *************

	if (vy[2] > vy[1])
	{
		dColor *line_stop_pointer	= buffer + vy[2] * Width();

		if (middle_x<vx[1])
		{
			linestart=middle_x;
			lineend=vx[1];

			line_u_start = middle_u;
			line_v_start = middle_v;

			add_start=(vx[2]-vx[0])/(vy[2]-vy[0]);
			add_end=(vx[2]-vx[1])/(vy[2]-vy[1]);

			u_line_add=(u[2]-u[0])/(vy[2]-vy[0]);
			v_line_add=(v[2]-v[0])/(vy[2]-vy[0]);
		}
		else
		{
			linestart=vx[1];
			lineend=middle_x;

			line_u_start = u[1];
			line_v_start = v[1];

			add_start=(vx[2]-vx[1])/(vy[2]-vy[1]);
			add_end=(vx[2]-vx[0])/(vy[2]-vy[0]);

			u_line_add=(u[2]-u[1])/(vy[2]-vy[1]);
			v_line_add=(v[2]-v[1])/(vy[2]-vy[1]);
		}
	
	// CLIP Y

		if (vy[1] < clp_top)
		{
			line_pointer = buffer + clp_top * Width();
			linestart	 += (clp_top - vy[1]) * add_start;
			lineend		 += (clp_top - vy[1]) * add_end;

			line_u_start += u_line_add * (clp_top-vy[1]);
			line_v_start += v_line_add * (clp_top-vy[1]);
		}

		if (vy[2]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineTextured(line_pointer,linestart>>16,lineend>>16,texture->Bits(),line_u_start,line_v_start,add_u,add_v);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
			
			line_u_start += u_line_add;
			line_v_start += v_line_add;
		}
	}	
}

void dBitmap::DrawFaceTexturedClut(dFace *face,dBitmap *texture,dClut2 *clut)
{
	if (face->vex[0]->z<0 || face->vex[1]->z<0 || face->vex[2]->z<0) return;
/*
	uint16 clp_left=clp_left+ScreenLeft(),clp_right=clp_right+ScreenLeft();
	uint16 clp_top=clp_top+ScreenTop(),clp_bottom=clp_bottom+ScreenTop();
*/

	int32 vx[3];
	int32 vy[3];
	int32 u[3];
	int32 v[3];
	
	vx[0]=face->vex[0]->x*65536;
	vy[0]=face->vex[0]->y;
	u[0]=face->u[0];
	v[0]=face->v[0];

	vx[1]=face->vex[1]->x*65536;
	vy[1]=face->vex[1]->y;
	u[1]=face->u[1];
	v[1]=face->v[1];

	vx[2]=face->vex[2]->x*65536;
	vy[2]=face->vex[2]->y;
	u[2]=face->u[2];
	v[2]=face->v[2];

	//sort by y hardco(re)ded bub-sort
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); swap(u[0],u[1]); swap(v[0],v[1]);}
	if (vy[1] > vy[2]) {	swap(vy[1],vy[2]); swap(vx[1],vx[2]); swap(u[1],u[2]); swap(v[1],v[2]);}
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); swap(u[0],u[1]); swap(v[0],v[1]);}

	if (vy[0] == vy[2]) return;

	int32 linestart,lineend;
	int32 add_start,add_end;
	int32 line_u_start,line_v_start;
	int32 u_line_add,v_line_add;
	int32 add_u,add_v;

	dColor *line_pointer = buffer + vy[0]	* Width();
	line_u_start = u[0];
	line_v_start = v[0];

	int32 middle_x=vx[0]+((vx[2]-vx[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));
	int32 middle_u=u[0]+((u[2]-u[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));
	int32 middle_v=v[0]+((v[2]-v[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));

	if (vx[1]>>16 == middle_x>>16) return;

	if (middle_x<vx[1])
	{
		add_u=(u[1]-middle_u)/(((vx[1]>>16)-(middle_x>>16)));
		add_v=(v[1]-middle_v)/(((vx[1]>>16)-(middle_x>>16)));
	}
	else
	{
		add_u=(middle_u-u[1])/(((middle_x>>16)-(vx[1]>>16)));
		add_v=(middle_v-v[1])/(((middle_x>>16)-(vx[1]>>16)));
	}

// ***** UPPER TRIANGLE *************
	
	if (vy[1] > vy[0])
	{
		linestart=lineend=vx[0];

		dColor *line_stop_pointer = buffer + vy[1] * Width();
	
		if (middle_x < vx[1])
		{
			add_start=(vx[2]-linestart)/(vy[2]-vy[0]);
			add_end=(vx[1]-lineend)/(vy[1]-vy[0]);

			u_line_add=(u[2]-u[0])/(vy[2]-vy[0]);
			v_line_add=(v[2]-v[0])/(vy[2]-vy[0]);
		}
		else
		{
			add_start=(vx[1]-linestart)/(vy[1]-vy[0]);
			add_end=(vx[2]-lineend)/(vy[2]-vy[0]);

			u_line_add=(u[1]-u[0])/(vy[1]-vy[0]);
			v_line_add=(v[1]-v[0])/(vy[1]-vy[0]);
		}
	
	// CLIP Y

		if (vy[0] < clp_top)
		{
			line_pointer += (clp_top - vy[0]) * Width();
			linestart	 += (clp_top - vy[0]) * add_start;
			lineend		 += (clp_top - vy[0]) * add_end;

			line_u_start += u_line_add * (clp_top-vy[0]);
			line_v_start += v_line_add * (clp_top-vy[0]);
		}

		if (vy[1]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineTexturedClut(line_pointer,linestart>>16,lineend>>16,texture->Bits(),line_u_start,line_v_start,add_u,add_v,clut);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
			
			line_u_start += u_line_add;
			line_v_start += v_line_add;
		}
	}

// ***** BOTTOM TRIANGLE *************

	if (vy[2] > vy[1])
	{
		dColor *line_stop_pointer	= buffer + vy[2] * Width();

		if (middle_x<vx[1])
		{
			linestart=middle_x;
			lineend=vx[1];

			line_u_start = middle_u;
			line_v_start = middle_v;

			add_start=(vx[2]-vx[0])/(vy[2]-vy[0]);
			add_end=(vx[2]-vx[1])/(vy[2]-vy[1]);

			u_line_add=(u[2]-u[0])/(vy[2]-vy[0]);
			v_line_add=(v[2]-v[0])/(vy[2]-vy[0]);
		}
		else
		{
			linestart=vx[1];
			lineend=middle_x;

			line_u_start = u[1];
			line_v_start = v[1];

			add_start=(vx[2]-vx[1])/(vy[2]-vy[1]);
			add_end=(vx[2]-vx[0])/(vy[2]-vy[0]);

			u_line_add=(u[2]-u[1])/(vy[2]-vy[1]);
			v_line_add=(v[2]-v[1])/(vy[2]-vy[1]);
		}
	
	// CLIP Y

		if (vy[1] < clp_top)
		{
			line_pointer = buffer + clp_top * Width();
			linestart	 += (clp_top - vy[1]) * add_start;
			lineend		 += (clp_top - vy[1]) * add_end;

			line_u_start += u_line_add * (clp_top-vy[1]);
			line_v_start += v_line_add * (clp_top-vy[1]);
		}

		if (vy[2]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineTexturedClut(line_pointer,linestart>>16,lineend>>16,texture->Bits(),line_u_start,line_v_start,add_u,add_v,clut);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
			
			line_u_start += u_line_add;
			line_v_start += v_line_add;
		}
	}	
}

void dBitmap::DrawFaceFlat(dFace *face,dColor col)
{
	if (face->vex[0]->z<0 || face->vex[1]->z<0 || face->vex[2]->z<0) return;
/*

	uint16 clp_left=clp_left+ScreenLeft(),clp_right=clp_right+ScreenLeft();
	uint16 clp_top=clp_top+ScreenTop(),clp_bottom=clp_bottom+ScreenTop();
*/
	int32 vx[3];
	int32 vy[3];
	
	vx[0]=face->vex[0]->x*65536;
	vy[0]=face->vex[0]->y;

	vx[1]=face->vex[1]->x*65536;
	vy[1]=face->vex[1]->y;

	vx[2]=face->vex[2]->x*65536;
	vy[2]=face->vex[2]->y;

	//sort by y hardco(re)ded bub-sort
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); }
	if (vy[1] > vy[2]) {	swap(vy[1],vy[2]); swap(vx[1],vx[2]); }
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); }

	if (vy[0] == vy[2]) return;

	int32 linestart,lineend;
	int32 add_start,add_end;

	dColor *line_pointer = buffer + vy[0]	* Width();

	int32 middle_x=vx[0]+((vx[2]-vx[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));

	if (vx[1]>>16 == middle_x>>16) return;

// ***** UPPER TRIANGLE *************
	
	if (vy[1] > vy[0])
	{
		linestart=lineend=vx[0];

		dColor *line_stop_pointer = buffer + vy[1] * Width();
	
		if (middle_x < vx[1])
		{
			add_start=(vx[2]-linestart)/(vy[2]-vy[0]);
			add_end=(vx[1]-lineend)/(vy[1]-vy[0]);
		}
		else
		{
			add_start=(vx[1]-linestart)/(vy[1]-vy[0]);
			add_end=(vx[2]-lineend)/(vy[2]-vy[0]);
		}
	
	// CLIP Y

		if (vy[0] < clp_top)
		{
			line_pointer += (clp_top - vy[0]) * Width();
			linestart	 += (clp_top - vy[0]) * add_start;
			lineend		 += (clp_top - vy[0]) * add_end;
		}

		if (vy[1]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineFlat(line_pointer,linestart>>16,lineend>>16,col);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
		}
	}

// ***** BOTTOM TRIANGLE *************

	if (vy[2] > vy[1])
	{
		dColor *line_stop_pointer	= buffer + vy[2] * Width();

		if (middle_x<vx[1])
		{
			linestart=middle_x;
			lineend=vx[1];

			add_start=(vx[2]-vx[0])/(vy[2]-vy[0]);
			add_end=(vx[2]-vx[1])/(vy[2]-vy[1]);
		}
		else
		{
			linestart=vx[1];
			lineend=middle_x;

			add_start=(vx[2]-vx[1])/(vy[2]-vy[1]);
			add_end=(vx[2]-vx[0])/(vy[2]-vy[0]);
		}
	
	// CLIP Y

		if (vy[1] < clp_top)
		{
			line_pointer = buffer + clp_top * Width();
			linestart	 += (clp_top - vy[1]) * add_start;
			lineend		 += (clp_top - vy[1]) * add_end;
		}

		if (vy[2]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineFlat(line_pointer,linestart>>16,lineend>>16,col);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
		}
	}	
}

void dBitmap::DrawFaceFlatClut2(dFace *face,dColor color,dClut2 *clut)
{
	if (face->vex[0]->z<0 || face->vex[1]->z<0 || face->vex[2]->z<0) return;
/*

	uint16 clp_left=clp_left+ScreenLeft(),clp_right=clp_right+ScreenLeft();
	uint16 clp_top=clp_top+ScreenTop(),clp_bottom=clp_bottom+ScreenTop();
*/
	int32 vx[3];
	int32 vy[3];
	
	vx[0]=face->vex[0]->x*65536;
	vy[0]=face->vex[0]->y;

	vx[1]=face->vex[1]->x*65536;
	vy[1]=face->vex[1]->y;

	vx[2]=face->vex[2]->x*65536;
	vy[2]=face->vex[2]->y;

	//sort by y hardco(re)ded bub-sort
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); }
	if (vy[1] > vy[2]) {	swap(vy[1],vy[2]); swap(vx[1],vx[2]); }
	if (vy[0] > vy[1]) {	swap(vy[0],vy[1]); swap(vx[0],vx[1]); }

	if (vy[0] == vy[2]) return;

	int32 linestart,lineend;
	int32 add_start,add_end;

	dColor *line_pointer = buffer + vy[0]	* Width();

	int32 middle_x=vx[0]+((vx[2]-vx[0])/(vy[2]-vy[0])*(vy[1]-vy[0]));

	if (vx[1]>>16 == middle_x>>16) return;

// ***** UPPER TRIANGLE *************
	
	if (vy[1] > vy[0])
	{
		linestart=lineend=vx[0];

		dColor *line_stop_pointer = buffer + vy[1] * Width();
	
		if (middle_x < vx[1])
		{
			add_start=(vx[2]-linestart)/(vy[2]-vy[0]);
			add_end=(vx[1]-lineend)/(vy[1]-vy[0]);
		}
		else
		{
			add_start=(vx[1]-linestart)/(vy[1]-vy[0]);
			add_end=(vx[2]-lineend)/(vy[2]-vy[0]);
		}
	
	// CLIP Y

		if (vy[0] < clp_top)
		{
			line_pointer += (clp_top - vy[0]) * Width();
			linestart	 += (clp_top - vy[0]) * add_start;
			lineend		 += (clp_top - vy[0]) * add_end;
		}

		if (vy[1]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineFlatClut2(line_pointer,linestart>>16,lineend>>16,color,clut);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
		}
	}

// ***** BOTTOM TRIANGLE *************

	if (vy[2] > vy[1])
	{
		dColor *line_stop_pointer	= buffer + vy[2] * Width();

		if (middle_x<vx[1])
		{
			linestart=middle_x;
			lineend=vx[1];

			add_start=(vx[2]-vx[0])/(vy[2]-vy[0]);
			add_end=(vx[2]-vx[1])/(vy[2]-vy[1]);
		}
		else
		{
			linestart=vx[1];
			lineend=middle_x;

			add_start=(vx[2]-vx[1])/(vy[2]-vy[1]);
			add_end=(vx[2]-vx[0])/(vy[2]-vy[0]);
		}
	
	// CLIP Y

		if (vy[1] < clp_top)
		{
			line_pointer = buffer + clp_top * Width();
			linestart	 += (clp_top - vy[1]) * add_start;
			lineend		 += (clp_top - vy[1]) * add_end;
		}

		if (vy[2]>clp_bottom + 1)
			line_stop_pointer = buffer + (clp_bottom + 1) * Width();

		while (line_pointer<line_stop_pointer)
		{
			HorizLineFlatClut2(line_pointer,linestart>>16,lineend>>16,color,clut);
			linestart += add_start;
			lineend	 += add_end;
			line_pointer += Width();
		}
	}	
}

