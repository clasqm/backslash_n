#ifndef _dOBJECT_
#define _dOBJECT_

#include "dPolygon.h"
#include "dFace.h"
#include "dVertex.h"

class dObject
{
friend class dApp; // !! TEMPORARY !!
public:
	dObject(float x,float y,float z);
	virtual ~dObject();
	virtual dObject *Clone();

	bool Load(BDirectory *dir,const char *filename);
	bool LoadPoints(BDirectory *dir,const char *filename);
	bool LoadLines(BDirectory *dir,const char *filename);

//	void New(uint32 num_vectors,vertex_struct *vertex_list,uint32 num_faces,face_struct *face_list);
	void WrapTexture();

	void PrecalcPointNorms();
	
//	void Save(char *filename);
	void RotateTo(float xa,float ya,float za);
	void RotateXTo(float xa);
	void RotateYTo(float ya);
	void RotateZTo(float za);

	void RotateTo(dVertex *normal);
	
	void RotateBy(float xa,float ya,float za);
	void RotateXBy(float xa);
	void RotateYBy(float ya);
	void RotateZBy(float za);

	void MoveTo(float x,float y,float z);
	void MoveXTo(float x);
	void MoveYTo(float y);
	void MoveZTo(float z);

	dVertex Position();
	float X();
	float Y();
	float Z();

	bool MorphSpin(float amount,dObject *target);
	bool MorphSpin2(float time,dObject *target);
	bool MorphScale(float amount,dObject *target);
	bool MorphDissolve(float amount,dObject *target);

	virtual void Project();
	virtual void ProjectSpin(float amount);

	int32 PolygonCount();
	int32 MaterialCount();
	int32 VertexCount();
	
	dPolygon *Polygon(int32 index);
	dVertex *Vertex(int32 index);
	dVertex *ProjectedVertex(int32 index);
	dVertex *RotatedNormal(int32 index);

protected:
	bool is_line_duplicate(dVertex *v1,dVertex *v2);
	uint16 rand();
	
	int32 rand_seed;
	
	void rebuild(float xa,float ya,float za,float x,float y,float z);
// ––––––––––––––––––––––
	dPolygon **polygon; // the polygons

	dMaterial *materials;

	dVertex *vertices; // the vertices
	dVertex *rotated_vertices;

	dVertex *normals; // the vertices normals, used for envmapping and gouraud shading
	dVertex *rotated_normals; // the rotated vertices normals, used for envmapping and gouraud shading
// ––––––––––––––––––––––
	int32 polygon_count; // # of polygons
	int32 material_count; // # of materials
	int32 vertex_count; // # of vertices

	float t[16];
	float xa,ya,za;
	float xpos,ypos,zpos;
};

// inline definitions

inline dVertex *dObject::RotatedNormal(int32 index)
{
	return rotated_normals + index;
}

inline dPolygon *dObject::Polygon(int32 index)
{
	return polygon[index];
}

inline dVertex *dObject::Vertex(int32 index)
{
	return vertices + index;
}

inline dVertex *dObject::ProjectedVertex(int32 index)
{
	return rotated_vertices + index;
}

inline int32 dObject::PolygonCount()
{
	return polygon_count;
}

inline int32 dObject::MaterialCount()
{
	return material_count;
}

inline int32 dObject::VertexCount()
{
	return vertex_count;
}

inline bool dObject::is_line_duplicate(dVertex *v1,dVertex *v2)
{
	for (int32 i=0; i < polygon_count ; i++)
		if (polygon[i]->VertexCount() == 2)
			if ((polygon[i]->vex[0] == v1 && polygon[i]->vex[1] == v2) || (polygon[i]->vex[1] == v1 && polygon[i]->vex[0] == v2)) return true;
	return false;
}

inline float dObject::X()
{
	return t[3];
}

inline float dObject::Y()
{
	return t[7];
}

inline float dObject::Z()
{
	return t[11];
}

inline dVertex dObject::Position()
{
	return dVertex(X(),Y(),Z());
}

inline void dObject::RotateBy(float x,float y,float z)
{
	RotateTo(xa+x,ya+y,za+z);
}

inline void dObject::RotateXBy(float x)
{
	RotateXTo(xa+x);
}

inline void dObject::RotateYBy(float y)
{
	RotateYTo(ya+y);
}

inline void dObject::RotateZBy(float z)
{
	RotateZTo(za+z);
}

inline uint16 dObject::rand()
{
	rand_seed = 16807*(rand_seed % 127773) - (rand_seed/127773)*2836;
	if (rand_seed < 0) rand_seed+=0x7fffffff;
	return rand_seed;
}

#endif // _dOBJECT_
