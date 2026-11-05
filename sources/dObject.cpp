#include "dObject.h"
#include "WarIFFChunk.h"
#include "dMaterial.h"
#include "dFace.h"
#include "dLine.h"
#include "dDot.h"
#include "dVertex.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

dObject *dObject::Clone()
{
	dObject *obj = new dObject(xpos,ypos,zpos);

	rand_seed=1;
	obj->vertex_count = vertex_count;
	obj->polygon_count = polygon_count;
	obj->material_count = material_count;

	obj->vertices = new dVertex[vertex_count];
	obj->rotated_vertices = new dVertex[vertex_count];
	obj->normals = new dVertex[vertex_count];
	obj->rotated_normals = new dVertex[vertex_count];

	obj->polygon = new dPolygon*[polygon_count];
	obj->materials = new dMaterial[material_count];

	memcpy(obj->t,t,16 * sizeof(float));
	
	obj->xa = xa;
	obj->ya = ya;
	obj->za = za;
	
	// copying the vertices
	
	for (int32 i=0;i<vertex_count;i++)
		obj->vertices[i] = vertices[i];

	// copying the normals
	
	for (int32 i=0;i<vertex_count;i++)
		obj->normals[i] = normals[i];

	// copying the materials (the textures aren't copied, just the reference)
	
	for (int32 i = 0; i < material_count; i++)
	{
		obj->materials[i].type = materials[i].type;
		obj->materials[i].color = materials[i].color;
		obj->materials[i].texture = materials[i].texture;
		obj->materials[i].clut = materials[i].clut;
	}

	// copying the polygons
	
	for (int32 i=0;i<polygon_count;i++)
	{
		switch (polygon[i]->VertexCount())
		{
			case 1:
			obj->polygon[i] = new dDot();
			break;
			case 2:
			obj->polygon[i] = new dLine();
			break;
			case 3:
			obj->polygon[i] = new dFace();
			break;
		}

		for (int32 j = 0; j < polygon[i]->VertexCount();j++)
		{
			obj->polygon[i]->vex[j] = obj->rotated_vertices + (polygon[i]->vex[j] - rotated_vertices);
			obj->polygon[i]->u[j] = polygon[i]->u[j];
			obj->polygon[i]->v[j] = polygon[i]->v[j];
		}
		obj->polygon[i]->material = obj->materials + (polygon[i]->material - materials);
	}
	return obj;
}

_EXPORT dObject::dObject(float x,float y,float z)
{
	rand_seed=1;
	rebuild(0,0,0,x,y,z);
	normals=NULL;
	rotated_normals=NULL;
	vertex_count = 0;
	vertices=NULL;
	rotated_vertices=NULL;
	polygon = NULL;
	polygon_count = 0;

	materials = new dMaterial();
	materials->type = MATERIAL_FLAT;
	materials->color = dColor(255);
	
	material_count = 1;
}

_EXPORT dObject::~dObject()
{
	// delete all the polygons
	while (polygon_count > 0)
	{
		polygon_count--;
		delete polygon[polygon_count];
	}
	if (polygon) delete []polygon;

	if (normals) delete []normals;
	if (rotated_normals) delete []rotated_normals;
	if (vertices) delete []vertices;
	if (rotated_vertices) delete []rotated_vertices;
}

bool dObject::LoadLines(BDirectory *dir,const char *filename)
{
// init the position
	MoveTo(0.0,0.0,0.0);

// open the file
	WarIFFChunk file(dir,filename,B_READ_ONLY);
	if (file.InitCheck() != B_OK)
	{
		fprintf(stderr,"error loading lightwave object: \"%s\"\n",filename);
		return false;
	}
	
//	************* Vertices

	WarIFFChunk points;
	off_t size;
	if (file.FindChunk('PNTS',&points) == B_ERROR) return false;
	points.GetSize(&size);
	vertex_count = size/12;

	vertices = new dVertex[vertex_count];
	rotated_vertices = new dVertex[vertex_count];
	normals = new dVertex[vertex_count];
	rotated_normals = new dVertex[vertex_count];

	for (int c=0;c<vertex_count;c++)
	{
		points.ReadFloat(&vertices[c].x);
		points.ReadFloat(&vertices[c].y);
		points.ReadFloat(&vertices[c].z);
		
		rotated_vertices[c] = vertices[c];
	}

//	************** POLYGONS

	WarIFFChunk polys;
	if (file.FindChunk('POLS',&polys) == B_ERROR) return false;

//	fprintf(stderr,"0x%x size == %d faces\n",size,size/10);
	polys.GetSize((off_t*)&size);

	// delete all the polygons
	while (polygon_count > 0)
		delete polygon[--polygon_count];
	if (polygon) delete []polygon;

	polygon=new dPolygon*[size];
	polygon_count = 0;

	uint16 tmp;

	uint32 read_bytes = 0;

	while (size > read_bytes)
	{
		read_bytes += polys.ReadUint16(&tmp);
		if (tmp == 3) // TRIANGLES
		{
			dVertex *v[3];
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
				v[j] = rotated_vertices + index;
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material

			if (!is_line_duplicate(v[0],v[1]))
			{
				polygon[polygon_count] = new dLine();
				polygon[polygon_count]->vex[0] = v[0];
				polygon[polygon_count]->vex[1] = v[1];
				polygon[polygon_count]->material = materials;
				polygon_count++;
			}
			if (!is_line_duplicate(v[0],v[2]))
			{
				polygon[polygon_count] = new dLine();
				polygon[polygon_count]->vex[0] = v[0];
				polygon[polygon_count]->vex[1] = v[2];
				polygon[polygon_count]->material = materials;
				polygon_count++;
			}
			if (!is_line_duplicate(v[2],v[1]))
			{
				polygon[polygon_count] = new dLine();
				polygon[polygon_count]->vex[0] = v[2];
				polygon[polygon_count]->vex[1] = v[1];
				polygon[polygon_count]->material = materials;
				polygon_count++;
			}
		}
		else if (tmp == 2) // LINES
		{
			polygon[polygon_count] = new dLine();
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
				polygon[polygon_count]->vex[j] = rotated_vertices + index;
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material
			polygon[polygon_count]->material = materials;
			polygon_count++;
		}
		else if (tmp == 1) // DOTS
		{
			polygon[polygon_count] = new dDot();
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
				polygon[polygon_count]->vex[j] = rotated_vertices + index;
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material
			polygon[polygon_count]->material = materials;
			polygon_count++;
		}
		else // UNSUPPORTED
		{
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material
		}
	}

//	set normals
	for (int i=0;i<polygon_count;i++)
		polygon[i]->CalcNormal();

	PrecalcPointNorms();
	
	return true;
}


bool dObject::Load(BDirectory *dir,const char *filename)
{
// init the position
	MoveTo(0.0,0.0,0.0);

// open the file
	WarIFFChunk file(dir,filename,B_READ_ONLY);
	if (file.InitCheck() != B_OK)
	{
		fprintf(stderr,"error loading lightwave object: \"%s\"\n",filename);
		return false;
	}
	
//	************* Vertices

	WarIFFChunk points;
	off_t size;
	if (file.FindChunk('PNTS',&points) == B_ERROR) return false;
	points.GetSize(&size);
	vertex_count = size/12;

	vertices = new dVertex[vertex_count];
	rotated_vertices = new dVertex[vertex_count];
	normals = new dVertex[vertex_count];
	rotated_normals = new dVertex[vertex_count];

	for (int c=0;c<vertex_count;c++)
	{
		points.ReadFloat(&vertices[c].x);
		points.ReadFloat(&vertices[c].y);
		points.ReadFloat(&vertices[c].z);
		
		rotated_vertices[c] = vertices[c];
	}

//	************** POLYGONS

	WarIFFChunk polys;
	if (file.FindChunk('POLS',&polys) == B_ERROR) return false;

//	fprintf(stderr,"0x%x size == %d faces\n",size,size/10);
	polys.GetSize((off_t*)&size);

	// delete all the polygons
	while (polygon_count > 0)
		delete polygon[--polygon_count];
	if (polygon) delete []polygon;

	polygon=new dPolygon*[size / 3];
	polygon_count = 0;

	uint16 tmp;

	uint32 read_bytes = 0;

	while (size > read_bytes)
	{
		read_bytes += polys.ReadUint16(&tmp);
		if (tmp == 3) // TRIANGLES
		{
			polygon[polygon_count] = new dFace();
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
				polygon[polygon_count]->vex[j] = rotated_vertices + index;
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material

			polygon[polygon_count]->material = materials;
			polygon_count++;
		}
		else if (tmp == 2) // LINES
		{
			polygon[polygon_count] = new dLine();
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
				polygon[polygon_count]->vex[j] = rotated_vertices + index;
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material

			polygon[polygon_count]->material = materials;
			polygon_count++;
		}
		else if (tmp == 1) // DOTS
		{
			polygon[polygon_count] = new dDot();
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
				polygon[polygon_count]->vex[j] = rotated_vertices + index;
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material

			polygon[polygon_count]->material = materials;
			polygon_count++;
		}
		else // UNSUPPORTED
		{
			for (uint16 j=0;j<tmp;j++)
			{
				uint16 index;
				read_bytes += polys.ReadUint16(&index);
			}
			uint16 material;
			read_bytes += polys.ReadUint16(&material); // material
		}
	}

//	set normals
	for (int i=0;i<polygon_count;i++)
		polygon[i]->CalcNormal();

	PrecalcPointNorms();
	
	return true;
}

bool dObject::LoadPoints(BDirectory *dir,const char *filename)
{
// init the position
	MoveTo(0.0,0.0,0.0);

// open the file
	WarIFFChunk file(dir,filename,B_READ_ONLY);
	if (file.InitCheck() != B_OK)
	{
		fprintf(stderr,"error loading lightwave object: \"%s\"\n",filename);
		return false;
	}
	
//	************* Vertices

	WarIFFChunk points;
	off_t size;
	if (file.FindChunk('PNTS',&points) == B_ERROR) return false;
	points.GetSize(&size);
	vertex_count = size/12;

	// delete all the polygons
	while (polygon_count > 0)
		delete polygon[polygon_count - 1];
	if (polygon) delete []polygon;

	if (vertices) delete []vertices;
	if (rotated_vertices) delete []rotated_vertices;
	if (normals) delete []normals;
	normals = NULL;
	if (rotated_normals) delete []rotated_normals;
	rotated_normals = NULL;

	vertices = new dVertex[vertex_count];
	rotated_vertices = new dVertex[vertex_count];
	normals = new dVertex[vertex_count];
	rotated_normals = new dVertex[vertex_count];

	for (int c=0;c<vertex_count;c++)
	{
		points.ReadFloat(&vertices[c].x);
		points.ReadFloat(&vertices[c].y);
		points.ReadFloat(&vertices[c].z);
		
		rotated_vertices[c] = vertices[c];
	}

// we don't have to read the polygons since we're only using the points
	polygon_count = vertex_count;

	polygon = new dPolygon*[polygon_count];
	for (int32 i = 0;i<vertex_count;i++)
	{
		polygon[i] = new dDot();
		polygon[i]->vex[0] = &rotated_vertices[i];
	}

	return true;
}

_EXPORT void dObject::RotateTo(dVertex *normal)
{
	float x=normal->y*sin(-za) - normal->x*cos(-za);
	float y=normal->y*cos(-za) + normal->x*sin(-za);

	rebuild(-atan(y/normal->z),-atan(x/normal->z),za,xpos,ypos,zpos); // z: 0 grader
}

void dObject::rebuild(float xang,float yang,float zang,float x,float y,float z)
{
	xa=xang;
	ya=yang;
	za=zang;
	
	xpos=x;
	ypos=y;
	zpos=z;

	// round x-axis
	float x_t[16] ={1,	0,			0,			0,
					0,	cos(xa),	-sin(xa),	0,
					0,	sin(xa),	cos(xa),	0,
					0,	0,			0,			1};

	// round y-axis
	float y_t[16] ={cos(ya),	0,	sin(ya),	0,
					0,			1,	0,			0,
					-sin(ya),	0,	cos(ya),	0,
					0,			0,	0,			1};

	// round z-axis
	float z_t[16] ={cos(za),	-sin(za),	0,	0,
					sin(za),	cos(za),	0,	0,
					0,			0,			1,	0,
					0,			0,			0,	1};

	// the camera
	float c_t[16] ={1,	0,	0,	xpos,
					0,	1,	0,	ypos,
					0,	0,	1,	zpos,
					0,	0,	0,	1};

	// invert the x-axis
	float i_t[16] ={-1,	0,	0,	0,
					0,	1,	0,	0,
					0,	0,	1,	0,
					0,	0,	0,	1};


	float t1[16];
	float t2[16];

	uint8 a,b;


	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t1[b*4+a] = z_t[b*4+0]*x_t[0*4+a] +	z_t[b*4+1]*x_t[1*4+a] + z_t[b*4+2]*x_t[2*4+a] + z_t[b*4+3]*x_t[3*4+a];

/*
	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t1[b*4+a] = x_t[b*4+0]*y_t[0*4+a] +	x_t[b*4+1]*y_t[1*4+a] + x_t[b*4+2]*y_t[2*4+a] + x_t[b*4+3]*y_t[3*4+a];
*/



/*
	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t2[b*4+a] = z_t[b*4+0]*t1[0*4+a] + z_t[b*4+1]*t1[1*4+a] + z_t[b*4+2]*t1[2*4+a] + z_t[b*4+3]*t1[3*4+a];
*/
/*
	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t2[b*4+a] = y_t[b*4+0]*t1[0*4+a] + y_t[b*4+1]*t1[1*4+a] + y_t[b*4+2]*t1[2*4+a] + y_t[b*4+3]*t1[3*4+a];
*/
	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t2[b*4+a] = t1[b*4+0]*y_t[0*4+a] + t1[b*4+1]*y_t[1*4+a] + t1[b*4+2]*y_t[2*4+a] + t1[b*4+3]*y_t[3*4+a];


	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t1[b*4+a] = c_t[b*4+0]*t2[0*4+a] + c_t[b*4+1]*t2[1*4+a] + c_t[b*4+2]*t2[2*4+a] + c_t[b*4+3]*t2[3*4+a];

/*
	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t2[b*4+a] = t1[b*4+0]*c_t[0*4+a] + t1[b*4+1]*c_t[1*4+a] + t1[b*4+2]*c_t[2*4+a] + t1[b*4+3]*c_t[3*4+a];
*/

	for (int b=0;b<4;b++)
		for (int a=0;a<4;a++)
			t[b*4+a] = t1[b*4+0]*i_t[0*4+a] + t1[b*4+1]*i_t[1*4+a] + t1[b*4+2]*i_t[2*4+a] + t1[b*4+3]*i_t[3*4+a];

}

_EXPORT void dObject::MoveTo(float x,float y,float z)
{
	rebuild(0,0,0,x,y,z);
}

_EXPORT void dObject::MoveXTo(float x)
{
	rebuild(0,0,0,x,ypos,zpos);
}

_EXPORT void dObject::MoveYTo(float y)
{
	rebuild(0,0,0,xpos,y,zpos);
}

_EXPORT void dObject::MoveZTo(float z)
{
	rebuild(0,0,0,xpos,ypos,z);
}

_EXPORT void dObject::RotateTo(float x,float y,float z)
{
	rebuild(x,y,z,xpos,ypos,zpos);
};

_EXPORT void dObject::RotateXTo(float x)
{
	rebuild(x,ya,za,xpos,ypos,zpos);
};

_EXPORT void dObject::RotateYTo(float y)
{
	rebuild(xa,y,za,xpos,ypos,zpos);
};

_EXPORT void dObject::RotateZTo(float z)
{
	rebuild(xa,ya,z,xpos,ypos,zpos);
};

_EXPORT void dObject::Project()
{
	for (int c=0;c<vertex_count;c++)
	{
		vertices[c].Rotate(t,&rotated_vertices[c]); // rotate
		normals[c].RotateSimple(t,&rotated_normals[c]);
		rotated_vertices[c].Project(); // project
	}
}

_EXPORT void dObject::ProjectSpin(float amount)
{
	for (int32 i=0;i<vertex_count;i++)
	{
		vertices[i].Rotate(t,&rotated_vertices[i]); // rotate
		rotated_vertices[i].RotateZ((rotated_vertices[i].x*rotated_vertices[i].y + sin(amount))*amount,&rotated_vertices[i]);
		normals[i].RotateSimple(t,&rotated_normals[i]);
		rotated_vertices[i].Project(); // project
	}
}

_EXPORT void dObject::PrecalcPointNorms()
{
	dVertex normal;
	uint16 num_faces;

	for (int i=0;i<vertex_count;i++)
	{
		num_faces=0;
		normal.SetTo(0,0,0);
		for (int c=0;c<polygon_count;c++)
			if (polygon[c]->VertexCount() == 3 && (polygon[c]->vex[0]==&rotated_vertices[i] || polygon[c]->vex[1]==&rotated_vertices[i] || polygon[c]->vex[2]==&rotated_vertices[i]))
			{
				normal += polygon[c]->Normal();
				num_faces++;
			}
		if (num_faces) normal/=num_faces;
		normals[i]=normal;
	}
}

bool dObject::MorphSpin(float amount,dObject *target)
{
	if (target->vertex_count != vertex_count) return false;
	
	for (int32 i=0;i<vertex_count;i++)
		vertices[i].RotateZ(vertices[i].z*amount,&target->vertices[i]);
	return true;
}

bool dObject::MorphSpin2(float time,dObject *target)
{
	if (target->vertex_count != vertex_count) return false;
	
	for (int32 i=0;i<vertex_count;i++)
		vertices[i].RotateZ(((vertices[i].z*2.1+10+sin(2+time*4))+vertices[i].z)*sin(time*3)*.3,&target->vertices[i]);
	return true;
}

bool dObject::MorphScale(float amount,dObject *target)
{
	if (target->vertex_count != vertex_count) return false;
	
	for (int32 i=0;i<vertex_count;i++)
		target->vertices[i] = vertices[i] * amount;
	return true;
}

bool dObject::MorphDissolve(float amount,dObject *target)
{
	if (target->vertex_count != vertex_count) return false;
	
	for (int32 i=0;i<vertex_count;i++)
		target->vertices[i] = vertices[i] + dVertex((rand()/65535.0 - .5) * amount,(rand()/65535.0 - .5) * amount,(rand()/65535.0 - .5) * amount);
	return true;
}

void dObject::WrapTexture()
{
	float x_min,x_max,y_min,y_max;
	
	x_min = Vertex(0)->x;
	x_max = Vertex(0)->x;
	y_min = Vertex(0)->y;
	y_max = Vertex(0)->y;

	for (int32 i=1;i<VertexCount();i++)
	{
		if (Vertex(i)->x < x_min) x_min = Vertex(i)->x;
		if (Vertex(i)->x > x_max) x_max = Vertex(i)->x;
		if (Vertex(i)->y < y_min) y_min = Vertex(i)->y;
		if (Vertex(i)->y > y_max) y_max = Vertex(i)->y;
	}

	float span_x = x_max - x_min;
	float span_y = y_max - y_min;

	for (int32 i=0;i<PolygonCount();i++)
	{
		for (int32 j=0;j<Polygon(i)->VertexCount();j++)
		{
			Polygon(i)->u[j] = ((Polygon(i)->vex[j]->x - x_min)/ span_x * 511.0) * 65536.0;
			Polygon(i)->v[j] = ((Polygon(i)->vex[j]->y - y_min)/ span_y * 511.0) * 65536.0;
		}
	}
}