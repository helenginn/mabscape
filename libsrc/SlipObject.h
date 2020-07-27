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

#include <vec3.h>
#include <mat4x4.h>

typedef struct
{
	GLfloat pos[3];
	GLfloat normal[3];
	GLfloat color[4];
	GLfloat extra[4];
	GLfloat tex[2];
} Vertex;

typedef struct
{
	GLuint index[3];
	GLfloat z;
} IndexTrio;


class SlipGL;

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
	
	void setDisabled(bool dis);
	
	void addToVertices(vec3 add);
	
	void recolour(double red, double green, double blue,
	              std::vector<Vertex> *vs = NULL);
	void changeProgram(std::string &v, std::string &f);
	vec3 centroid();
	vec3 nearestVertex(vec3 pos);
	
	void changeMidPoint(double x, double y);
	void setHighlighted(bool highlighted);
	void setSelected(bool selected);
	void reorderIndices();
	void boundaries(vec3 *min, vec3 *max);
	bool intersects(double x, double y, double *z);
protected:
	void addVertex(float v1, float v2, float v3);
	void addIndex(GLuint i);
	void resize(double scale);
	void setSelectable(bool selectable);

	std::vector<Vertex> _vertices;
	std::vector<GLuint> _indices;
	std::vector<Vertex> _unselectedVertices;

	bool _central;
private:
	GLuint addShaderFromString(GLuint program, GLenum type, std::string str);
	void checkErrors();
	void rebindProgram();
	void deletePrograms();
	void bindTextures();
	void addToVertexArray(vec3 add, std::vector<Vertex> *vs);

	static bool index_behind_index(IndexTrio one, IndexTrio two);
	static bool index_in_front_of_index(IndexTrio one, IndexTrio two);

	std::string _vString;
	std::string _fString;
	std::string _random;
	GLuint _program;
	GLuint _bufferID;
	GLuint _vbo;
	GLuint _renderType;
	GLuint _uModel;
	GLuint _uProj;
	GLuint _uTime;
	std::vector<GLuint> _textures;
	std::vector<IndexTrio> _temp; // stores with model mat
	std::string _name;
	mat4x4 _model;
	mat4x4 _proj;
	
	bool _extra;
	bool _disabled;
	bool _selected;
	bool _highlighted;
	bool _selectable;
	bool _backToFront;
};

#endif
