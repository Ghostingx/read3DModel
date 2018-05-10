
//

#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>


#include<string>
#include<vector>
#include<iostream>
#pragma warning(disable:4996)
using namespace std;
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#define     PI    3.1415926


typedef struct Vertex {
	float x, y, z;
} Vertex;

typedef struct Face {
	Face(void) : nverts(0), verts(0) {};
	int nverts;
	Vertex **verts;
	float normal[3];
} Face;

typedef struct Mesh {
	Mesh(void) : nverts(0), verts(0), nfaces(0), faces(0) {};
	int nverts;
	Vertex *verts;
	int nfaces;
	Face *faces;

	vector<Vertex>point;

} Mesh;
typedef struct TextureCoor {
	float s, t;
}TextureCoor;

typedef struct NormalVector {
	float u, v, w;
}NormalVector;

typedef struct FaceObj {
	FaceObj(void) : nverts(0), verts(0){};
	int nverts;
	vector<int> verts;
	vector<int> vt;
	vector<int> vn;
	float normal[3];
} FaceObj;

typedef struct MeshObj {
	MeshObj(void) : nverts(0), nfaces(0){}
	int nverts;
	int nfaces;
	vector<Vertex> verts;
	vector<TextureCoor> vt;
	vector<NormalVector> vn;
	vector<FaceObj> faces;
}MeshObj;

typedef struct HE_face
{
	struct HE_edge *edge;  // one of the half-edges bordering the face

} HE_face;
typedef struct HE_vert
{

	float x;
	float y;
	float z;

	struct HE_edge *edge;  // one of the half-edges emantating from the vertex

}HE_vert;
typedef struct HE_edge
{

	struct HE_vert *vert;   // vertex at the end of the half-edge
	struct HE_edge *pair;   // oppositely oriented adjacent half-edge
	struct HE_face *face;   // face the half-edge borders
	struct HE_edge *next;   // next half-edge around the face
    struct HE_vert *midV;
    struct TextureCoor *vt;
    struct NormalVector *vn;

}HE_edge;
typedef struct HE_mesh {
	vector<TextureCoor> vt;
	vector<NormalVector> vn;
	vector<HE_vert *> vert;
	vector<HE_edge *> edge;
	vector<HE_face *> face;
    ~HE_mesh(){

        vt.clear();
        vn.clear();
        vert.clear();
        edge.clear();
        face.clear();
    }
}HE_mesh;

const char  *  filename ;
static int objoroff = 0;


// GLUT variables

static int GLUTwindow = 0;
static int GLUTwindow_height = 800;
static int GLUTwindow_width = 800;
static int GLUTmouse[2] = { 0, 0 };
static int GLUTbutton[3] = { 0, 0, 0 };
static int GLUTarrows[4] = { 0, 0, 0, 0 };
static int GLUTmodifiers = 0;




// Display variables

static int scaling = 0;
static int translating = 0;
static int rotating = 0;
static float scale = 1.0;
static float center[3] = { 0.0, 0.0, 0.0 };
static float rotation[3] = { 0.0, 0.0, 0.0 };
static float translation[3] = { 0.0, 0.0, -4.0 };



// Mesh variables

static Mesh *mesh = NULL;
static MeshObj * meshObj = NULL;
static HE_mesh * HEMesh = NULL;
static HE_mesh * newHE = NULL;

HE_vert * findVert(const HE_mesh *HEMesh,float x,float y,float z) {
	for (int i = 0; i < HEMesh->vert.size(); i++)
	{
		if (fabs(x - HEMesh->vert[i]->x) <= 1E-6 && fabs(y - HEMesh->vert[i]->y) <= 1E-6 && fabs(z - HEMesh->vert[i]->z) <= 1E-6)
			return HEMesh->vert[i];
	}
	return NULL;

}

int findVertIndex(const HE_mesh *HEMesh,float x,float y,float z) {
	for (int i = 0; i < HEMesh->vert.size(); i++)
	{
		if (fabs(x - HEMesh->vert[i]->x) <= 1E-6 && fabs(y - HEMesh->vert[i]->y) <= 1E-6 && fabs(z - HEMesh->vert[i]->z) <= 1E-6)
			return i;
	}
	return -1;

}

HE_edge * findPairEdge(const HE_mesh * _HEMesh,const HE_edge *_edge) {
	HE_edge * _pairEdge = NULL;
	for (int i = 0; i < _HEMesh->edge.size();i++)
	{
        if(_edge->next->vert == _HEMesh->edge[i]->vert && _HEMesh->edge[i]->next->vert == _edge->vert)
		_pairEdge = _HEMesh->edge[i];
	}
    return _pairEdge;
}

void toHE(const MeshObj * mesh){
	HEMesh = new HE_mesh();
	for (int i = 0; i < mesh->nverts; i++)
	{
		HE_vert * vert = new HE_vert();
		vert->x = mesh->verts[i].x;
		vert->y = mesh->verts[i].y;
		vert->z = mesh->verts[i].z;
		HEMesh->vert.push_back(vert);
	}

    for(int i = 0;i <mesh->vt.size(); i++){
        HEMesh->vt.push_back(mesh->vt[i]);
    }
    for(int i = 0;i <mesh->vn.size(); i++){
        HEMesh->vn.push_back(mesh->vn[i]);
    }
    int vt_size = HEMesh->vt.size();
    int vn_size = HEMesh->vn.size();

	for (int i = 0; i < mesh->nfaces; i++)
	{
		HE_face * face = new HE_face();
		HE_edge * lastEdge = new HE_edge();
		HE_edge * firstEdge = lastEdge;
		HEMesh->edge.push_back(firstEdge);
		lastEdge->face = face;
		lastEdge->vert =  findVert(HEMesh,mesh->verts[mesh->faces[i].verts[0]].x,mesh->verts[mesh->faces[i].verts[0]].y,mesh->verts[mesh->faces[i].verts[0]].z);
            lastEdge->vert->edge = lastEdge;
        lastEdge->vt = vt_size > 0 ? &(HEMesh->vt[mesh->faces[i].vt[0]]) : NULL;
        lastEdge->vn = vn_size > 0 ? &(HEMesh->vn[mesh->faces[i].vn[0]]) : NULL;
        HE_edge * edge;
		for (int j = 1; j < mesh->faces[i].nverts; j++)
		{
			edge = new HE_edge();
			lastEdge->next = edge;
			edge->face = face;
			edge->vert = findVert(HEMesh,mesh->verts[mesh->faces[i].verts[j]].x,mesh->verts[mesh->faces[i].verts[j]].y,mesh->verts[mesh->faces[i].verts[j]].z);
            edge->vert->edge = edge;
            edge->vt = vt_size > 0 ? &(HEMesh->vt[mesh->faces[i].vt[j]]) : NULL;
            edge->vn = vn_size > 0 ? &(HEMesh->vn[mesh->faces[i].vn[j]]) : NULL;
			HEMesh->edge.push_back(edge);
			lastEdge = edge;
		}
		lastEdge->next = firstEdge;
		face->edge = firstEdge;
		HEMesh->face.push_back(face);
	}
	for (int i = 0; i < HEMesh->edge.size(); i++)
	{
		HEMesh->edge[i]->pair = findPairEdge(HEMesh,HEMesh->edge[i]);
	}

}

void loopSubdivision(HE_mesh* HEmesh,HE_mesh *submesh){
  //  for(int i= 0;i<HEmesh->vert.size();i++){
  //      submesh->vert.push_back(HEmesh->vert[i]);
  //  }

    for(int i = 0;i<HEmesh->face.size();i++){
        HE_edge * edge = HEmesh->face[i]->edge;
        do{
        if(edge->pair){
            if(edge->pair->midV){
                edge->midV = edge->pair->midV;
            }else{
                
                edge->midV = new HE_vert();
                edge->midV->x = (edge->vert->x + edge->pair->vert->x) * 3 / 8.0 + (edge->next->next->vert->x + edge->pair->next->next->vert->x) / 8.0;
                edge->midV->y = (edge->vert->y + edge->pair->vert->y) * 3 / 8.0 + (edge->next->next->vert->y + edge->pair->next->next->vert->y) / 8.0;
                edge->midV->z = (edge->vert->z + edge->pair->vert->z) * 3 / 8.0 + (edge->next->next->vert->z + edge->pair->next->next->vert->z) / 8.0;
            }   
        }else{
            edge->midV = new HE_vert();
            edge->midV->x = (edge->vert->x + edge->next->vert->x) / 2.0;    
            edge->midV->y = (edge->vert->y + edge->next->vert->y) / 2.0;    
            edge->midV->z = (edge->vert->z + edge->next->vert->z) / 2.0;    
        }
        edge = edge->next;
        }while(edge != HEmesh->face[i]->edge);       
    }
    for(int i = 0;i<HEmesh->vert.size();i++){
        HE_edge *edge = HEmesh->vert[i]->edge;
        float X,Y,Z;
        X=Y=Z=0.0;
        int n = 0;
        do{
            X += edge->pair->vert->x;
            Y += edge->pair->vert->y;
            Z += edge->pair->vert->z;
            n++;
        }while(edge != HEmesh->vert[i]->edge);
        float bata = (5.0/8 -((3.0/8 + cos(PI * 2.0 / n) / 4) * (3.0/8 + cos(PI * 2.0 /n) / 4))) / n;
        HE_vert *vert = new HE_vert();
        vert->x = bata * X + HEmesh->vert[i]->x * (1 - bata * n);
        vert->y = bata * Y + HEmesh->vert[i]->y * (1 - bata * n);
        vert->z = bata * Z + HEmesh->vert[i]->z * (1 - bata * n);
        submesh->vert.push_back(vert);
    }

    for(int i = 0;i<HEmesh->face.size();i++){//create four faces 
        HE_vert* midv[3];
        HE_edge* edge = HEmesh->face[i]->edge;
        for(int  j = 0;j<3;j++){
            HE_vert* v;
            if((v=findVert(submesh,edge->midV->x,edge->midV->y,edge->midV->z)) == NULL)
            {
                midv[j] = new HE_vert();
                midv[j]->x = edge->midV->x;
                midv[j]->y = edge->midV->y;
                midv[j]->z = edge->midV->z;
                submesh->vert.push_back(midv[j]);
            }else
            {
                midv[j] = v;
            }  
            edge =edge->next;
        }
        
        HE_face *face;
        HE_edge* newEdge[3];
        face = new HE_face();
        newEdge[0] = new HE_edge();
        newEdge[0]->vert = submesh->vert[findVertIndex(HEmesh,edge->vert->x,edge->vert->y,edge->vert->z)]; 
        newEdge[0]->face = face;
        newEdge[1] = new HE_edge();
        newEdge[1]->vert = midv[0];
        newEdge[1]->face = face;
        newEdge[2] = new HE_edge();
        newEdge[2]->vert = midv[2];
        newEdge[2]->face = face;
        
        newEdge[0]->next = newEdge[1];
        newEdge[1]->next = newEdge[2];
        newEdge[2]->next = newEdge[0];

        face->edge = newEdge[0];
        submesh->edge.push_back(newEdge[0]);
        submesh->edge.push_back(newEdge[1]);
        submesh->edge.push_back(newEdge[2]);
        submesh->face.push_back(face);
        
        edge = edge->next;

        face = new HE_face();
        newEdge[0] = new HE_edge();
        newEdge[0]->vert = submesh->vert[findVertIndex(HEmesh,edge->vert->x,edge->vert->y,edge->vert->z)]; 
        newEdge[0]->face = face;
        newEdge[1] = new HE_edge();
        newEdge[1]->vert = midv[1];
        newEdge[1]->face = face;
        newEdge[2] = new HE_edge();
        newEdge[2]->vert = midv[0];
        newEdge[2]->face = face;
        
        newEdge[0]->next = newEdge[1];
        newEdge[1]->next = newEdge[2];
        newEdge[2]->next = newEdge[0];

        face->edge = newEdge[0];
        submesh->edge.push_back(newEdge[0]);
        submesh->edge.push_back(newEdge[1]);
        submesh->edge.push_back(newEdge[2]);
        submesh->face.push_back(face);

        
        edge = edge->next;

        face = new HE_face();
        newEdge[0] = new HE_edge();
        newEdge[0]->vert = submesh->vert[findVertIndex(HEmesh,edge->vert->x,edge->vert->y,edge->vert->z)]; 
        newEdge[0]->face = face;
        newEdge[1] = new HE_edge();
        newEdge[1]->vert = midv[2];
        newEdge[1]->face = face;
        newEdge[2] = new HE_edge();
        newEdge[2]->vert = midv[1];
        newEdge[2]->face = face;
        
        newEdge[0]->next = newEdge[1];
        newEdge[1]->next = newEdge[2];
        newEdge[2]->next = newEdge[0];

        face->edge = newEdge[0];
        submesh->edge.push_back(newEdge[0]);
        submesh->edge.push_back(newEdge[1]);
        submesh->edge.push_back(newEdge[2]);
        submesh->face.push_back(face);
        
        edge = edge->next;

        face = new HE_face();
        newEdge[0] = new HE_edge();
        midv[0]->edge = newEdge[0];
        newEdge[0]->vert = midv[0]; 
        newEdge[0]->face = face;
        newEdge[1] = new HE_edge();
        midv[1]->edge = newEdge[1];
        newEdge[1]->vert = midv[1];
        newEdge[1]->face = face;
        newEdge[2] = new HE_edge();
        midv[2]->edge = newEdge[2];
        newEdge[2]->vert = midv[2];
        newEdge[2]->face = face;
        
        newEdge[0]->next = newEdge[1];
        newEdge[1]->next = newEdge[2];
        newEdge[2]->next = newEdge[0];

        face->edge = newEdge[0];
        submesh->edge.push_back(newEdge[0]);
        submesh->edge.push_back(newEdge[1]);
        submesh->edge.push_back(newEdge[2]);
        submesh->face.push_back(face);

    }

}



void ComputeNormal(MeshObj &meshobj, FaceObj &face) {
	face.normal[0] = face.normal[1] = face.normal[2] = 0;

	Vertex *v1 = &meshobj.verts[face.verts[face.nverts - 1]];
	for (int i = 0; i < face.nverts; i++) {
		Vertex *v2 = &meshobj.verts[face.verts[i]];
		face.normal[0] += (v1->y - v2->y) * (v1->z + v2->z);
		face.normal[1] += (v1->z - v2->z) * (v1->x + v2->x);
		face.normal[2] += (v1->x - v2->x) * (v1->y + v2->y);
		v1 = v2;
	}

	// Normalize normal for face
	float squared_normal_length = 0.0;
	squared_normal_length += face.normal[0] * face.normal[0];
	squared_normal_length += face.normal[1] * face.normal[1];
	squared_normal_length += face.normal[2] * face.normal[2];
	float normal_length = sqrt(squared_normal_length);
	if (normal_length > 1.0E-6) {
		face.normal[0] /= normal_length;
		face.normal[1] /= normal_length;
		face.normal[2] /= normal_length;
	}
}

MeshObj * ReadObjFile(const char * filename ) {
	FILE *fp;
	if (!(fp = fopen(filename, "r"))) {
		fprintf(stderr, "Unable to open file %s\n", filename);
		return 0;
	}

	// Allocate mesh structure
	meshObj = new MeshObj();
	if (!meshObj) {
		fprintf(stderr, "Unable to allocate memory for file %s\n", filename);
		fclose(fp);
		return 0;
	}

	Vertex * v;
	NormalVector * vn;
	TextureCoor * vt;
	FaceObj * f;
	char buffer[1024];
	int line_count = 0;
	while (fgets(buffer, 1023, fp)) {
		// Increment line counter
		line_count++;

		// Skip white space
		char *bufferp = buffer;
		while (isspace(*bufferp)) bufferp++;

		// Skip blank lines and comments
		if (*bufferp == '#') continue;
		if (*bufferp == '\0') continue;
		if (strncmp(bufferp,"vt",2) == 0)
		{
			bufferp++;
			bufferp++;
			bufferp++;
			vt = new TextureCoor();
			if (sscanf(bufferp,"%f%f",&(vt->s), &(vt->t)) != 2)
			{
				fprintf(stderr, "Syntax error with vertex texture coordinates on line %d in file %s\n", line_count, filename);
				fclose(fp);
				return NULL;
			}
			meshObj->vt.push_back(*vt);
		}
		else if(strncmp(bufferp, "vn",2) == 0)
		{
			bufferp++;
			bufferp++;
			bufferp++;
			vn = new NormalVector();
			if (sscanf(bufferp, "%f%f%f", &(vn->u), &(vn->v), &(vn->w)) != 3)
			{
				fprintf(stderr, "Syntax error with vertex normal vector on line %d in file %s\n", line_count, filename);
				fclose(fp);
				return NULL;
			}
			meshObj->vn.push_back(*vn);
		}
		else if (strncmp(bufferp, "v",1) == 0)
		{
			bufferp++;
			bufferp++;
			v = new Vertex();
			if (sscanf(bufferp, "%f%f%f", &(v->x), &(v->y), &(v->z)) != 3)
			{
				fprintf(stderr, "Syntax error with vertex coordinates on line %d in file %s\n", line_count, filename);
				fclose(fp); return NULL;
			}
			meshObj->verts.push_back(*v);
			meshObj->nverts++;
		}
		else if (strncmp(bufferp, "f",1) == 0)
		{
			bufferp++;
			bufferp++;
			char * vv;
			f = new FaceObj();
			if (meshObj->vt.size() > 0 && meshObj->vn.size() > 0)
			{
				int v[3];
				vv = strtok(bufferp, " ");
				while (vv)
				{
					if ((sscanf(vv, "%d/%d/%d", &(v[0]), &(v[1]), &(v[2]))) == 3)
					{
						f->verts.push_back(v[0] - 1);
						f->vt.push_back(v[1] - 1);
						f->vn.push_back(v[2] - 1);

						f->nverts++;
					}
					vv = strtok(NULL," ");
				}

				ComputeNormal(*meshObj, *f);
				meshObj->faces.push_back(*f);
				meshObj->nfaces++;
			}
			else if (meshObj->vt.size() == 0 && meshObj->vn.size() > 0)
			{
				int v[2];
				vv = strtok(bufferp, " ");
				while (vv)
				{
					if ((sscanf(vv, "%d//%d", &(v[0]), &(v[1]), &(v[2]))) == 2)
					{
						f->verts.push_back(v[0] - 1);
						f->vn.push_back(v[1] - 1);

						f->nverts++;
					}
					vv = strtok(NULL, " ");
				}
				ComputeNormal(*meshObj, *f);
				meshObj->faces.push_back(*f);
				meshObj->nfaces++;
			}
			else if (meshObj->vt.size() == 0 && meshObj->vn.size() == 0)
			{
				int v;
				vv = strtok(bufferp, " ");
				while (vv)
				{
					if ((sscanf(vv, "%d", &v)) == 1)
					{
						f->verts.push_back(v - 1);

						f->nverts++;
					}
					vv = strtok(NULL, " ");
				}
				ComputeNormal(*meshObj,*f);
				meshObj->faces.push_back(*f);
				meshObj->nfaces++;
			}
		}
		else
		{

		}
	}

	return meshObj;
}







void GLUTRedrawObj(void)
{
	// Setup viewing transformation
	glLoadIdentity();
	glScalef(scale, scale, scale);
	glTranslatef(translation[0], translation[1], 0.0);

	// Set projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)GLUTwindow_width / (GLfloat)GLUTwindow_height, 0.1, 100.0);

	// Set camera transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(translation[0], translation[1], translation[2]);
	glScalef(scale, scale, scale);
	glRotatef(rotation[0], 1.0, 0.0, 0.0);
//	glRotatef(rotation[1], 0.0, 1.0, 0.0);
	glRotatef(rotation[2], 0.0, 0.0, 1.0);
	glTranslatef(-center[0], -center[1], -center[2]);

	// Clear window
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set lights
	static GLfloat light0_position[] = { 3.0, 4.0, 5.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	static GLfloat light1_position[] = { -3.0, -2.0, -3.0, 0.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	// Set material
	static GLfloat material[] = { 0.5, 1.0, 0.5, 0.5 };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
	Vertex *vert = NULL;
	NormalVector *vn = NULL;
	TextureCoor *vt = NULL;
	// Draw faces
	//for (int i = 0; i < meshObj->nfaces; i++) {
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	for (int i = 0; i < newHE->face.size(); i++) {
       // int n = newHE->face.size();
       // material[0] = 1.0-i*1.0/n;
       // material[1] = 1.0-i*1.0/n;
       // material[2] = 1.0-i*1.0/n;
        //material[3] = 0.5;//{ 0.5, 1.0, 0.5, 0.5 };
	    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
       // glColor3f(1.0-1.0/(i+1),1.0-1.0/(i+1),1.0-1.0/(i+1));
		FaceObj& face = meshObj->faces[i];
       // HE_face * f = HEMesh->face[i];
        HE_face * f = newHE->face[i];
		glBegin(GL_POLYGON);
/*
		for (int j = 0; j < face.nverts; j++) {
			if (meshObj->vn.size() > 0) {
				vn = &meshObj->vn[face.verts[j]];
				glNormal3f(vn->u, vn->v, vn->w);
			}
			else
			{
				glNormal3fv(face.normal);
			}
			if (meshObj->vt.size() > 0)
			{
				vt = &meshObj->vt[face.verts[j]];
				glTexCoord2f(vt->s, vt->t);
			}

			vert = &meshObj->verts[face.verts[j]];
			//glVertex3f(vert->x, vert->y, vert->z);
		}*/
            HE_edge *e  =f->edge;
            do{
                //if(HEMesh->vt.size() > 0)
                //    glTexCoord2f(e->vt->s,e->vt->t);
                //if(HEMesh->vn.size() > 0)
                //    glNormal3f(e->vn->u,e->vn->v,e->vn->w);
                glVertex3f(e->vert->x,e->vert->y,e->vert->z);      
                e = e->next; 
            }while(e!=f->edge);
		glEnd();
	}

	// Swap buffers
	glutSwapBuffers();
}


void GLUTMainLoopObj(void)
{
	// Compute bounding box
	float bbox[2][3] = { { 1.0E30F, 1.0E30F, 1.0E30F },{ -1.0E30F, -1.0E30F, -1.0E30F } };
	for (int i = 0; i < meshObj->nverts; i++) {
		Vertex& vert = meshObj->verts[i];
		if (vert.x < bbox[0][0]) bbox[0][0] = vert.x;
		else if (vert.x > bbox[1][0]) bbox[1][0] = vert.x;
		if (vert.y < bbox[0][1]) bbox[0][1] = vert.y;
		else if (vert.y > bbox[1][1]) bbox[1][1] = vert.y;
		if (vert.z < bbox[0][2]) bbox[0][2] = vert.z;
		else if (vert.z > bbox[1][2]) bbox[1][2] = vert.z;
	}

	// Setup initial viewing scale
	float dx = bbox[1][0] - bbox[0][0];
	float dy = bbox[1][1] - bbox[0][1];
	float dz = bbox[1][2] - bbox[0][2];
	scale = 2.0 / sqrt(dx*dx + dy * dy + dz * dz);

	// Setup initial viewing center
	center[0] = 0.5 * (bbox[1][0] + bbox[0][0]);
	center[1] = 0.5 * (bbox[1][1] + bbox[0][1]);
	center[2] = 0.5 * (bbox[1][2] + bbox[0][2]);

	// Run main loop -- never returns
	glutMainLoop();
}

////////////////////////////////////////////////////////////
// OFF FILE READING CODE
////////////////////////////////////////////////////////////

Mesh *ReadOffFile(const char *filename)
{
	int i;

	// Open file
	FILE *fp;
	if (!(fp = fopen(filename, "r"))) {
		fprintf(stderr, "Unable to open file %s\n", filename);
		return 0;
	}

	// Allocate mesh structure
	Mesh *mesh = new Mesh();
	if (!mesh) {
		fprintf(stderr, "Unable to allocate memory for file %s\n", filename);
		fclose(fp);
		return 0;
	}

	// Read file
	int nverts = 0;
	int nfaces = 0;
	int nedges = 0;
	int line_count = 0;
	char buffer[1024];
	while (fgets(buffer, 1023, fp)) {
		// Increment line counter
		line_count++;

		// Skip white space
		char *bufferp = buffer;
		while (isspace(*bufferp)) bufferp++;

		// Skip blank lines and comments
		if (*bufferp == '#') continue;
		if (*bufferp == '\0') continue;

		// Check section
		if (nverts == 0) {
			// Read header
			if (!strstr(bufferp, "OFF")) {
				// Read mesh counts
				if ((sscanf(bufferp, "%d%d%d", &nverts, &nfaces, &nedges) != 3) || (nverts == 0)) {
					fprintf(stderr, "Syntax error reading header on line %d in file %s\n", line_count, filename);
					fclose(fp);
					return NULL;
				}

				// Allocate memory for mesh
				mesh->verts = new Vertex[nverts];
				assert(mesh->verts);
				mesh->faces = new Face[nfaces];
				assert(mesh->faces);
			}
		}
		else if (mesh->nverts < nverts) {
			// Read vertex coordinates
			Vertex& vert = mesh->verts[mesh->nverts++];
			if (sscanf(bufferp, "%f%f%f", &(vert.x), &(vert.y), &(vert.z)) != 3) {
				fprintf(stderr, "Syntax error with vertex coordinates on line %d in file %s\n", line_count, filename);
				fclose(fp);
				return NULL;
			}
		}
		else if (mesh->nfaces < nfaces) {
			// Get next face
			Face& face = mesh->faces[mesh->nfaces++];

			// Read number of vertices in face
			bufferp = strtok(bufferp, " \t");
			if (bufferp) face.nverts = atoi(bufferp);
			else {
				fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
				fclose(fp);
				return NULL;
			}

			// Allocate memory for face vertices
			face.verts = new Vertex *[face.nverts];
			assert(face.verts);

			// Read vertex indices for face
			for (i = 0; i < face.nverts; i++) {
				bufferp = strtok(NULL, " \t");
				if (bufferp) face.verts[i] = &(mesh->verts[atoi(bufferp)]);
				else {
					fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
					fclose(fp);
					return NULL;
				}
			}

			// Compute normal for face
			face.normal[0] = face.normal[1] = face.normal[2] = 0;
			Vertex *v1 = face.verts[face.nverts - 1];
			for (i = 0; i < face.nverts; i++) {
				Vertex *v2 = face.verts[i];
				face.normal[0] += (v1->y - v2->y) * (v1->z + v2->z);
				face.normal[1] += (v1->z - v2->z) * (v1->x + v2->x);
				face.normal[2] += (v1->x - v2->x) * (v1->y + v2->y);
				v1 = v2;
			}

			// Normalize normal for face
			float squared_normal_length = 0.0;
			squared_normal_length += face.normal[0] * face.normal[0];
			squared_normal_length += face.normal[1] * face.normal[1];
			squared_normal_length += face.normal[2] * face.normal[2];
			float normal_length = sqrt(squared_normal_length);
			if (normal_length > 1.0E-6) {
				face.normal[0] /= normal_length;
				face.normal[1] /= normal_length;
				face.normal[2] /= normal_length;
			}
		}
		else {
			// Should never get here
			fprintf(stderr, "Found extra text starting at line %d in file %s\n", line_count, filename);
			break;
		}
	}

	// Check whether read all faces
	if (nfaces != mesh->nfaces) {
		fprintf(stderr, "Expected %d faces, but read only %d faces in file %s\n", nfaces, mesh->nfaces, filename);
	}

	// Close file
	fclose(fp);

	// Return mesh
	return mesh;
}



////////////////////////////////////////////////////////////
// GLUT USER INTERFACE CODE
////////////////////////////////////////////////////////////

void GLUTRedraw(void)
{
	// Setup viewing transformation
	glLoadIdentity();
	glScalef(scale, scale, scale);
	glTranslatef(translation[0], translation[1], 0.0);

	// Set projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)GLUTwindow_width / (GLfloat)GLUTwindow_height, 0.1, 100.0);

	// Set camera transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(translation[0], translation[1], translation[2]);
	glScalef(scale, scale, scale);
	glRotatef(rotation[0], 1.0, 0.0, 0.0);
	glRotatef(rotation[1], 0.0, 1.0, 0.0);
	glRotatef(rotation[2], 0.0, 0.0, 1.0);
	glTranslatef(-center[0], -center[1], -center[2]);

	// Clear window
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set lights
	static GLfloat light0_position[] = { 3.0, 4.0, 5.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	static GLfloat light1_position[] = { -3.0, -2.0, -3.0, 0.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	// Set material
	static GLfloat material[] = { 0.3, 1.0, 0.3, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);

	// Draw faces
	for (int i = 0; i < mesh->nfaces; i++) {
		Face& face = mesh->faces[i];
		glBegin(GL_POLYGON);
		glNormal3fv(face.normal);
		for (int j = 0; j < face.nverts; j++) {
			Vertex *vert = face.verts[j];
			glVertex3f(vert->x, vert->y, vert->z);
		}
		glEnd();
	}

	// Swap buffers
	glutSwapBuffers();
}



void GLUTStop(void)
{
	// Destroy window
	glutDestroyWindow(GLUTwindow);

	// Exit
	exit(0);
}



void GLUTResize(int w, int h)
{
	// Resize window
	glViewport(0, 0, w, h);

	// Remember window size
	GLUTwindow_width = w;
	GLUTwindow_height = h;

	// Redraw
	glutPostRedisplay();
}



void GLUTMotion(int x, int y)
{
	// Invert y coordinate
	y = GLUTwindow_height - y;

	// Process mouse motion event
	if (rotating) {
		// Rotate model
		rotation[0] += -0.5 * (y - GLUTmouse[1]);
		rotation[2] += 0.5 * (x - GLUTmouse[0]);
	}
	else if (scaling) {
		// Scale window
		scale *= exp(2.0 * (float)(x - GLUTmouse[0]) / (float)GLUTwindow_width);
	}
	else if (translating) {
		// Translate window
		translation[0] += 2.0 * (float)(x - GLUTmouse[0]) / (float)GLUTwindow_width;
		translation[1] += 2.0 * (float)(y - GLUTmouse[1]) / (float)GLUTwindow_height;
	}

	// Remember mouse position
	GLUTmouse[0] = x;
	GLUTmouse[1] = y;
}



void GLUTMouse(int button, int state, int x, int y)
{
	// Invert y coordinate
	y = GLUTwindow_height - y;

	// Process mouse button event
	rotating = (button == GLUT_LEFT_BUTTON);
	scaling = (button == GLUT_MIDDLE_BUTTON);
	translating = (button == GLUT_RIGHT_BUTTON);
	if (rotating || scaling || translating) glutIdleFunc(objoroff == 0 ? GLUTRedraw : GLUTRedrawObj);
	else glutIdleFunc(0);

	// Remember button state
	int b = (button == GLUT_LEFT_BUTTON) ? 0 : ((button == GLUT_MIDDLE_BUTTON) ? 1 : 2);
	GLUTbutton[b] = (state == GLUT_DOWN) ? 1 : 0;

	// Remember modifiers
	GLUTmodifiers = glutGetModifiers();

	// Remember mouse position
	GLUTmouse[0] = x;
	GLUTmouse[1] = y;
}



void GLUTSpecial(int key, int x, int y)
{
	// Invert y coordinate
	y = GLUTwindow_height - y;

	// Process keyboard button event

	// Remember mouse position
	GLUTmouse[0] = x;
	GLUTmouse[1] = y;

	// Remember modifiers
	GLUTmodifiers = glutGetModifiers();

	// Redraw
	glutPostRedisplay();
}



void GLUTKeyboard(unsigned char key, int x, int y)
{
	// Process keyboard button event
	switch (key) {
	case 'Q':
	case 'q':
		GLUTStop();
		break;

	case 27: // ESCAPE
		GLUTStop();
		break;
	}

	// Remember mouse position
	GLUTmouse[0] = x;
	GLUTmouse[1] = GLUTwindow_height - y;

	// Remember modifiers
	GLUTmodifiers = glutGetModifiers();
}



void GLUTInit(int *argc, char **argv)
{
	// Open window
	glutInit(argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(GLUTwindow_width, GLUTwindow_height);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // | GLUT_STENCIL
	GLUTwindow = glutCreateWindow("3Dviewer");

	// Initialize GLUT callback functions
	glutReshapeFunc(GLUTResize);
	if(objoroff == 1)
		glutDisplayFunc(GLUTRedrawObj);
	else if(0 == objoroff)
	{
		glutDisplayFunc(GLUTRedraw);
	}
	glutKeyboardFunc(GLUTKeyboard);
	glutSpecialFunc(GLUTSpecial);
	glutMouseFunc(GLUTMouse);
	glutMotionFunc(GLUTMotion);
	glutIdleFunc(0);

	// Initialize lights
	static GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	static GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glEnable(GL_LIGHT0);
	static GLfloat light1_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);

	// Initialize graphics modes
	glEnable(GL_DEPTH_TEST);
}



void GLUTMainLoop(void)
{
	// Compute bounding box
	float bbox[2][3] = { { 1.0E30F, 1.0E30F, 1.0E30F },{ -1.0E30F, -1.0E30F, -1.0E30F } };
	for (int i = 0; i < mesh->nverts; i++) {
		Vertex& vert = mesh->verts[i];
		if (vert.x < bbox[0][0]) bbox[0][0] = vert.x;
		else if (vert.x > bbox[1][0]) bbox[1][0] = vert.x;
		if (vert.y < bbox[0][1]) bbox[0][1] = vert.y;
		else if (vert.y > bbox[1][1]) bbox[1][1] = vert.y;
		if (vert.z < bbox[0][2]) bbox[0][2] = vert.z;
		else if (vert.z > bbox[1][2]) bbox[1][2] = vert.z;
	}

	// Setup initial viewing scale
	float dx = bbox[1][0] - bbox[0][0];
	float dy = bbox[1][1] - bbox[0][1];
	float dz = bbox[1][2] - bbox[0][2];
	scale = 2.0 / sqrt(dx*dx + dy * dy + dz * dz);

	// Setup initial viewing center
	center[0] = 0.5 * (bbox[1][0] + bbox[0][0]);
	center[1] = 0.5 * (bbox[1][1] + bbox[0][1]);
	center[2] = 0.5 * (bbox[1][2] + bbox[0][2]);

	// Run main loop -- never returns
	glutMainLoop();
}





////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////

int
ParseArgs(int argc, char **argv)
{
	// Innocent until proven guilty
	int print_usage = 0;

	// Parse arguments
	argc--; argv++;
	while (argc > 0) {
		if ((*argv)[0] == '-') {
			if (!strcmp(*argv, "-help")) { print_usage = 1; }
			else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
			argv++; argc--;
		}
		else {
			if (!filename) filename = *argv;
			else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
			argv++; argc--;
		}
	}

	// Check filename
	if (!filename || print_usage) {
		printf("Usage: offviewer <filename>\n");
		return 0;
	}

	// Return OK status
	return 1;
}

void showOff() {

}
void showObj() {

}

////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
	if (!ParseArgs(argc, argv)) exit(1);

	//filename = "cross.off";
	filename = "cube.obj";
	// Initialize GLUT



	// Parse program arguments
	if (strstr(filename,".off"))
	{
		objoroff = 0;
		GLUTInit(&argc, argv);
		mesh = ReadOffFile(filename);
		if (!mesh) exit(1);
		GLUTMainLoop();
	}
	else if (strstr(filename, ".obj"))
	{
		objoroff = 1;
		GLUTInit(&argc, argv);
		ReadObjFile(filename);
		toHE(meshObj);

        newHE = new HE_mesh();
        loopSubdivision(HEMesh,newHE);

        for(int i =0;i < HEMesh->face.size();i++){
            
            HE_face * f = HEMesh->face[i];

            HE_edge * e = f->edge;
            do{
                cout<<e->vert->x<<"\t";
                cout<<e->vert->y<<"\t";
                cout<<e->vert->z<<"\t";
                e = e->next;
            }
            while(e != f->edge);
            cout<<endl;
        }
		    GLUTMainLoopObj();
	}

	// Return success
	return 0;
}









