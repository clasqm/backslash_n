#ifndef _dCONTOUR_OBJECT_
#define _dCONTOUR_OBJECT_

#include "dPolygon.h"
#include "dFace.h"
#include "dVertex.h"
#include "dObject.h"
#include <OS.h>
#include <SupportDefs.h>

class dContourObject:public dObject
{
public:
	dContourObject(float x,float y,float z);
	virtual ~dContourObject();
	virtual dObject *Clone();

	bool Load(BDirectory *dir,const char *filename);

	virtual void Project();

protected:

	void update();
	bool find_triangles(dVertex *v1, dVertex *v2,int32 *face1,int32 *face2);

	int32 triangle_count;
	dFace **triangle;  // num_triangles
	bool *tri_visible; // num_triangles
	int32 *line_link; // num_lines * 2
};

// inline definitions


#endif // _dCONTOUR_OBJECT_
