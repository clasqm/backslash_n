#include "dContourObject.h"
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

dObject *dContourObject::Clone()
{
	dContourObject *obj = new dContourObject(xpos,ypos,zpos);

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

	//
	obj->triangle_count = triangle_count;
	obj->triangle=new dFace*[triangle_count];
	obj->tri_visible = new bool[triangle_count];
	obj->line_link = new int32[PolygonCount()*2];

	for(int32 i=0;i<triangle_count;i++)
	{
		obj->triangle[i] = new dFace();
		for (int32 j = 0; j < 3;j++)
			obj->triangle[i]->vex[j] = obj->rotated_vertices + (triangle[i]->vex[j] - rotated_vertices);
	}
	
	for(int32 i=0;i<PolygonCount()*2;i++)
		obj->line_link[i] = line_link[i];

	return obj;
}

_EXPORT dContourObject::dContourObject(float x,float y,float z):dObject(x,y,z)
{
}

_EXPORT dContourObject::~dContourObject()
{
}

bool dContourObject::Load(BDirectory *dir,const char *filename)
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

	triangle=new dFace*[size];
	triangle_count = 0;
	tri_visible = new bool[size];
	line_link = new int32[size*2];

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

			triangle[triangle_count] = new dFace();
			triangle[triangle_count]->vex[0] = v[0];
			triangle[triangle_count]->vex[1] = v[1];
			triangle[triangle_count]->vex[2] = v[2];
			triangle_count++;

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

	for(int32 i = 0;i<PolygonCount();i++)
		find_triangles(Polygon(i)->vex[0],Polygon(i)->vex[1],&line_link[i*2],&line_link[i*2+1]);

//	set normals
	for (int i=0;i<polygon_count;i++)
		polygon[i]->CalcNormal();

	PrecalcPointNorms();
	
	return true;
}

void dContourObject::Project()
{
	dObject::Project();
	update();
}


bool dContourObject::find_triangles(dVertex *v1, dVertex *v2,int32 *face1,int32 *face2)
{
	uint32 found = 0;
	for(int32 i=0;i<triangle_count;i++)
		if ((triangle[i]->vex[0] == v1 || triangle[i]->vex[1] == v1 || triangle[i]->vex[2] == v1) && (triangle[i]->vex[0] == v2 || triangle[i]->vex[1] == v2 || triangle[i]->vex[2] == v2))
			if (found)
			{
				*face2 = i;
				return true;
			}
			else
			{
				*face1 = i;
				found++;
			}

	if (found)
	{
		*face2 = *face1;
		return true;
	}
	return false;
}

void dContourObject::update()
{
	for(int32 i=0;i<triangle_count;i++)
		tri_visible[i] = triangle[i]->IsFront();

	for(int32 i=0;i<PolygonCount();i++)
	{
		((dLine*)Polygon(i))->SetVisible(tri_visible[line_link[i*2]] != tri_visible[line_link[i*2+1]]);
	}
}