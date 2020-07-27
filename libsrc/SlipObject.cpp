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

#include "SlipObject.h"
#include "SlipGL.h"
#include "charmanip.h"
#include <mat3x3.h>
#include <float.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <QImage>
#include "shaders/vImage.h"
#include "shaders/fImage.h"
#include "shaders/fWipe.h"

vec3 vec_from_pos(GLfloat *pos)
{
	vec3 tmpVec = make_vec3(pos[0], pos[1],
	                        pos[2]);

	return tmpVec;
}

void pos_from_vec(GLfloat *pos, vec3 v)
{
	pos[0] = v.x;
	pos[1] = v.y;
	pos[2] = v.z;
}


void SlipObject::addToVertexArray(vec3 add, std::vector<Vertex> *vs)
{
	for (size_t i = 0; i < vs->size(); i++)
	{
		(*vs)[i].pos[0] += add.x;
		(*vs)[i].pos[1] += add.y;
		(*vs)[i].pos[2] += add.z;
	}
}

void SlipObject::addToVertices(vec3 add)
{
	addToVertexArray(add, &_vertices);
	addToVertexArray(add, &_unselectedVertices);
}

SlipObject::SlipObject()
{
	_renderType = GL_TRIANGLES;
	_program = 0;
	_backToFront = false;
	_bufferID = 0;
	_vbo = 0;
	_uModel = 0;
	_extra = 0;
	_central = 0;
	_disabled = 0;
	_highlighted = 0;
	_selected = 0;
	_selectable = 0;
}

GLuint SlipObject::addShaderFromString(GLuint program, GLenum type, 
                                       std::string str)
{
	GLint length = str.length();

	const char *cstr = str.c_str();
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &cstr, &length);
	glCompileShader(shader);

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		char *log;

		/* get the shader info log */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log = (char *)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);

		/* print an error message and the info log */
		std::cout << "Shader: unable to compile: " << std::endl;
		std::cout << str << std::endl;
		std::cout << log << std::endl;
		free(log);

		glDeleteShader(shader);
		return 0;
	}

	glAttachShader(_program, shader);
	return shader;
}

void SlipObject::deletePrograms()
{
	glDeleteProgram(_program);
	_program = 0;
}

void SlipObject::initialisePrograms(std::string *v, std::string *f)
{
	if (v == NULL)
	{
		_vString = Pencil_vsh();
		v = &_vString;
	}

	if (f == NULL)
	{
		f = &fImage;
	}
	
	initializeOpenGLFunctions();

	GLint result;

	/* create program object and attach shaders */
	_program = glCreateProgram();

	addShaderFromString(_program, GL_VERTEX_SHADER, *v);
	addShaderFromString(_program, GL_FRAGMENT_SHADER, *f);

	glBindAttribLocation(_program, 0, "position");
	glBindAttribLocation(_program, 1, "normal");
	glBindAttribLocation(_program, 2, "color");

	if (!_extra)
	{
		glBindAttribLocation(_program, 3, "projection");
	}
	else
	{
		glBindAttribLocation(_program, 3, "extra");
	}

	if (_textures.size())
	{
		glBindAttribLocation(_program, 4, "tex");
	}

	/* link the program and make sure that there were no errors */
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &result);
	checkErrors();

	if (result == GL_FALSE)
	{
		std::cout << "sceneInit(): Program linking failed." << std::endl;

		/* delete the program */
		glDeleteProgram(_program);
		_program = 0;
	}

	glGenBuffers(1, &_bufferID);
	glGenBuffers(1, &_vbo);

	bindTextures();
	rebindProgram();
}

void SlipObject::bindTextures()
{
	_textures.resize(1);

	glDeleteTextures(1, &_textures[0]);
	glGenTextures(1, &_textures[0]);

	/*
	getImage()->bindToTexture(this);
	*/
}

void SlipObject::rebindProgram()
{
	glBindBuffer(GL_ARRAY_BUFFER, _bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo);

	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize(), iPointer(), GL_STATIC_DRAW);

	checkErrors();

	/* Vertices */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(0 * sizeof(float)));
	/* Normals */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(3 * sizeof(float)));

	/* Colours */
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(6 * sizeof(float)));

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(10 * sizeof(float)));

	checkErrors();

	if (_textures.size())
	{
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(14 * sizeof(float)));
	}

	checkErrors();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	if (_textures.size())
	{
		glEnableVertexAttribArray(4); 
	}
}

void SlipObject::checkErrors()
{
	GLenum err = glGetError();

	if (err != 0)
	{
		std::cout << "OUCH " << err << std::endl;
		
		switch (err)
		{
			case GL_INVALID_ENUM:
			std::cout << "Invalid enumeration" << std::endl;
			break;

			case GL_STACK_OVERFLOW:
			std::cout << "Stack overflow" << std::endl;
			break;

			case GL_STACK_UNDERFLOW:
			std::cout << "Stack underflow" << std::endl;
			break;

			case GL_OUT_OF_MEMORY:
			std::cout << "Out of memory" << std::endl;
			break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "Invalid framebuffer op" << std::endl;
			break;

			case GL_INVALID_VALUE:
			std::cout << "Invalid value" << std::endl;
			break;

			case GL_INVALID_OPERATION:
			std::cout << "Invalid operation" << std::endl;
			break;

		}
	}
}

void SlipObject::render(SlipGL *sender)
{
	if (_disabled)
	{
		return;
	}
	
	if (_program == 0)
	{
		initialisePrograms();
	}
	
	glUseProgram(_program);
	rebindProgram();
	
	checkErrors();

	_model = sender->getModel();
	const char *uniform_name = "model";
	_uModel = glGetUniformLocation(_program, uniform_name);
	glUniformMatrix4fv(_uModel, 1, GL_FALSE, &_model.vals[0]);

	_proj = sender->getProjection();
	uniform_name = "projection";
	_uProj = glGetUniformLocation(_program, uniform_name);
	glUniformMatrix4fv(_uProj, 1, GL_FALSE, &_proj.vals[0]);

	float time = sender->getTime();
	uniform_name = "time";
	_uTime = glGetUniformLocation(_program, uniform_name);
	glUniform1f(_uTime, time);

	if (_textures.size())
	{
		glBindTexture(GL_TEXTURE_2D, _textures[0]);
	}
	
	glDrawElements(_renderType, indexCount(), GL_UNSIGNED_INT, 0);
	checkErrors();
	
	glUseProgram(0);
}

void SlipObject::recolour(double red, double green, double blue,
                          std::vector<Vertex> *vs)
{
	if (vs == NULL)
	{
		vs = &_vertices;
	}
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		(*vs)[i].color[0] = red;
		(*vs)[i].color[1] = green;
		(*vs)[i].color[2] = blue;
		(*vs)[i].color[3] = 1.0;
	}
}

void SlipObject::resize(double scale)
{
	vec3 centre = centroid();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&pos, centre);
		vec3_mult(&pos, scale);
		vec3_add_to_vec3(&pos, centre);
		pos_from_vec(_vertices[i].pos, pos);
	}

}

void SlipObject::changeProgram(std::string &v, std::string &f)
{
	deletePrograms();
	initialisePrograms(&v, &f);
}

void SlipObject::setDisabled(bool dis)
{
	_disabled = dis;
}

vec3 SlipObject::centroid()
{
	vec3 sum = empty_vec3();

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		sum.x += _vertices[i].pos[0];
		sum.y += _vertices[i].pos[1];
		sum.z += _vertices[i].pos[2];
	}
	
	double scale = 1 / (double)_vertices.size();
	vec3_mult(&sum, scale);
	
	return sum;
}

void SlipObject::addVertex(float v1, float v2, float v3)
{
	Vertex v;
	memset(v.pos, 0, sizeof(Vertex));

	v.color[2] = 0.5;
	v.color[3] = 1;
	v.pos[0] = v1;
	v.pos[1] = v2;
	v.pos[2] = v3;
	_vertices.push_back(v);

}

void SlipObject::addIndex(GLuint i)
{
	_indices.push_back(i);
}

bool SlipObject::index_behind_index(IndexTrio one, IndexTrio two)
{
	return (one.z > two.z);
}

bool SlipObject::index_in_front_of_index(IndexTrio one, IndexTrio two)
{
	return (one.z < two.z);
}

void SlipObject::reorderIndices()
{
	if (_renderType == GL_LINES)
	{
		return;
	}

	_temp.resize(_indices.size() / 3);
	
	int count = 0;
	for (size_t i = 0; i < _indices.size(); i+=3)
	{
		int n = _indices[i];
		vec3 tmpVec = vec_from_pos(_vertices[n].pos);
		n = _indices[i + 1];
		vec3 tmpVec1 = vec_from_pos(_vertices[n].pos);
		n = _indices[i + 2];
		vec3 tmpVec2 = vec_from_pos(_vertices[n].pos);
		vec3_add_to_vec3(&tmpVec, tmpVec1);
		vec3_add_to_vec3(&tmpVec, tmpVec2);
		tmpVec = mat4x4_mult_vec(_model, tmpVec);
		_temp[count].z = tmpVec.z;
		_temp[count].index[0] = _indices[i];
		_temp[count].index[1] = _indices[i + 1];
		_temp[count].index[2] = _indices[i + 2];
		count++;
	}
	
	if (_backToFront)
	{
		std::sort(_temp.begin(), _temp.end(), index_behind_index);
	}
	else
	{
		std::sort(_temp.begin(), _temp.end(), index_in_front_of_index);
	}

	count = 0;

	for (size_t i = 0; i < _temp.size(); i++)
	{
		_indices[count + 0] = _temp[i].index[0];
		_indices[count + 1] = _temp[i].index[1];
		_indices[count + 2] = _temp[i].index[2];
		count += 3;
	}
}

vec3 SlipObject::nearestVertex(vec3 pos)
{
	double closest = FLT_MAX;
	Vertex *vClose = NULL;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex v = _vertices[i];
		vec3 diff = make_vec3(pos.x - v.pos[0],
		                      pos.y - v.pos[1],
		                      pos.z - v.pos[2]);

		if (abs(diff.x) > closest || abs(diff.y) > closest 
		    || abs(diff.z) > closest)
		{
			continue;
		}

		double length = vec3_length(diff);
		
		if (length < closest)
		{
			closest = length;
			vClose = &_vertices[i];
		}
	}

	vec3 finvec = vec_from_pos(vClose->pos);
	return finvec;
}

void SlipObject::boundaries(vec3 *min, vec3 *max)
{
	*min = make_vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	*max = make_vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex v = _vertices[i];
		if (v.pos[0] < min->x) min->x = v.pos[0];
		if (v.pos[1] < min->y) min->y = v.pos[1];
		if (v.pos[2] < min->z) min->z = v.pos[2];

		if (v.pos[0] > max->x) max->x = v.pos[0];
		if (v.pos[1] > max->y) max->y = v.pos[1];
		if (v.pos[2] > max->z) max->z = v.pos[2];
	}
}

void SlipObject::changeMidPoint(double x, double y)
{
	vec3 pos = centroid();
	double last = 1;

	vec3 model = mat4x4_mult_vec3(_model, pos, &last);
	vec3 proj = mat4x4_mult_vec3(_proj, model, &last);
	
	double newx = last * x / _proj.vals[0];
	double newy = last * y / _proj.vals[5];
	vec3 move = make_vec3(newx - model.x, newy - model.y, 0);

	mat3x3 rot = mat4x4_get_rot(_model);

	vec3 newpos = mat3x3_mult_vec(rot, move);

	addToVertices(newpos);
	
}

bool SlipObject::intersects(double x, double y, double *z)
{
	vec3 target = make_vec3(x, y, 0);
	bool found = false;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_vertices[i].pos);
		
		if (_central)
		{
			pos = centroid();
		}
		
		double last = 1;
		vec3 model = mat4x4_mult_vec3(_model, pos, &last);
		vec3 proj = mat4x4_mult_vec3(_proj, model, &last);
		
		vec3_mult(&proj, 1 / last);

		if (proj.x < -1 || proj.x > 1)
		{
			continue;
		}

		if (proj.y < -1 || proj.y > 1)
		{
			continue;
		}
		
		if (model.z > 0)
		{
			continue;
		}

		vec3 diff = vec3_subtract_vec3(proj, target);
		
		if (fabs(diff.x) < 0.04 && fabs(diff.y) < 0.04)
		{
			if (model.z > *z)
			{
				*z = model.z;
				found = true;
			}
		}
		
		if (_central)
		{
			break;
		}
	}
	
	return found;
}

void SlipObject::setSelectable(bool selectable)
{
	if (selectable)
	{
		_unselectedVertices = _vertices;
	}
	else
	{
		_unselectedVertices.clear();
	}

	_selectable = selectable;
}

void SlipObject::setHighlighted(bool highlighted)
{
	if (!_selectable)
	{
		return;
	}
	
	if (highlighted && !_highlighted && !_selected)
	{
		_vertices = _unselectedVertices;
		resize(1.3);
	}
	
	if (!highlighted && _highlighted && !_selected)
	{
		_vertices = _unselectedVertices;
	}

	_highlighted = highlighted;
}

void SlipObject::setSelected(bool selected)
{
	if (!_selected && selected)
	{
		recolour(0, 0, 1);
		
		if (!_highlighted)
		{
			resize(1.3);
		}
	}
	
	if ((_selected || _highlighted) && !selected)
	{
		_vertices = _unselectedVertices;
	}

	_highlighted = selected;
	_selected = selected;
}
