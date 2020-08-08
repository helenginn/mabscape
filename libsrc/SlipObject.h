// Slip n Slide
// Copyright (C) 2017-2018 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __Slip_SlipObject__
#define __Slip_SlipObject__

#include <QtGui/qopengl.h>
#include <QtGui/qopenglfunctions.h>

#include <GLObject.h>
#include <mutex>
#include <vec3.h>
#include <mat4x4.h>


typedef struct
{
	GLuint index[3];
	GLfloat z;
} IndexTrio;

inline vec3 vec_from_pos(GLfloat *pos)
{
	vec3 tmpVec = make_vec3(pos[0], pos[1],
	                        pos[2]);

	return tmpVec;
}

inline void pos_from_vec(GLfloat *pos, vec3 v)
{
	pos[0] = v.x;
	pos[1] = v.y;
	pos[2] = v.z;
}

class SlipGL;
class Mesh;

class SlipObject : public QOpenGLFunctions
{
public:
	SlipObject();
	virtual ~SlipObject() {};
	void initialisePrograms(std::string *v = NULL, std::string *f = NULL);
	virtual void render(SlipGL *sender);
	
	Vertex *vPointer()
	{
		return &_vertices[0];
	}
	
	Vertex vertex(size_t idx)
	{
		return _vertices[idx];
	}
	
	size_t vertexCount()
	{
		return _vertices.size();
	}
	
	void setVertex(size_t idx, Vertex v)
	{
		_vertices[idx] = v;
	}

	size_t vSize()
	{
		return sizeof(Vertex) * _vertices.size();
	}

	GLuint *iPointer()
	{
		return &_indices[0];
	}

	size_t iSize()
	{
		return sizeof(GLuint) * _indices.size();
	}
	
	GLuint texture(size_t i)
	{
		return _textures[i];
	}
	
	size_t indexCount()
	{
		return _indices.size();
	}
	
	std::string name()
	{
		return _name;
	}
	
	void setName(std::string name)
	{
		_name = name;
	}
	
	bool isDisabled()
	{
		return _disabled;
	}
	
	void setModel(mat4x4 model)
	{
		_model = model;
	}
	
	void lockMutex()
	{
		_mut.lock();
	}
	
	bool tryLockMutex()
	{
		return _mut.try_lock();
	}
	
	void unlockMutex()
	{
		_mut.unlock();
	}
	
	void setDisabled(bool dis);
	
	void addToVertices(vec3 add);
	
	bool collapseCommonVertices();
	void recolour(double red, double green, double blue,
	              std::vector<Vertex> *vs = NULL);
	void setAlpha(double alpha);
	void changeProgram(std::string &v, std::string &f);
	vec3 centroid();
	vec3 randomVertex();
	vec3 nearestVertex(vec3 pos, bool useMesh = false);
	vec3 nearestVertexNearNormal(vec3 pos, vec3 normal, bool *isBehind);
	Vertex *nearestVertexPtr(vec3 pos, bool useMesh);
	
	void changeMidPoint(double x, double y);
	void setHighlighted(bool highlighted);
	void setSelected(bool selected);
	void reorderIndices();
	void boundaries(vec3 *min, vec3 *max);
	bool intersects(double x, double y, double *z);
	void writeObjFile(std::string filename);
	double envelopeRadius();
	double averageRadius();
	Mesh *makeMesh();
	
	bool hasMesh()
	{
		return (_mesh != NULL);
	}
	
	Mesh *mesh()
	{
		return _mesh;
	}

	bool pointInside(vec3 point);
	void colourOutlayBlack();
	void changeToLines();
	void changeToTriangles();
	virtual void triangulate();
	
	void remove()
	{
		_remove = true;
	}
	
	bool shouldRemove()
	{
		return _remove;
	}

	
	void clearMesh();

	void setColour(double red, double green, double blue)
	{
		_red = red;
		_green = green;
		_blue = blue;
		recolour(red, green, blue);
		recolour(red, green, blue, &_unselectedVertices);
	}

	void resize(double scale, bool unselected = false);
protected:
	bool polygonIncludes(vec3 point, GLuint *trio);
	vec3 rayTraceToPlane(vec3 point, GLuint *trio, vec3 dir,
	                     bool *backwards);
	void addVertex(float v1, float v2, float v3);
	void addIndex(GLuint i);
	void addIndices(GLuint i1, GLuint i2, GLuint i3);
	virtual void calculateNormals(bool flip = false);
	void setSelectable(bool selectable);
	void fixCentroid(vec3 centre);

	std::vector<Vertex> _vertices;
	std::vector<GLuint> _indices;
	std::vector<Vertex> _unselectedVertices;

	double _red;
	double _green;
	double _blue;

	bool _central;
	GLuint _renderType;
	std::string _vString;
	std::string _fString;
private:
	GLuint addShaderFromString(GLuint program, GLenum type, std::string str);
	void checkErrors();
	void rebindProgram();
	void deletePrograms();
	void bindTextures();
	void addToVertexArray(vec3 add, std::vector<Vertex> *vs);

	static bool index_behind_index(IndexTrio one, IndexTrio two);
	static bool index_in_front_of_index(IndexTrio one, IndexTrio two);

	std::string _random;
	GLuint _program;
	GLuint _bufferID;
	GLuint _vbo;
	GLuint _uModel;
	GLuint _uProj;
	GLuint _uTime;
	std::vector<GLuint> _textures;
	std::vector<IndexTrio> _temp; // stores with model mat
	std::string _name;
	mat4x4 _model;
	mat4x4 _proj;
	Mesh *_mesh;
	std::mutex _mut;
	
	bool _extra;
	bool _remove;
	bool _disabled;
	bool _selected;
	bool _highlighted;
	bool _selectable;
	bool _backToFront;
};

#endif
