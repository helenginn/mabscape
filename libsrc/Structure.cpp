// abmap
// Copyright (C) 2019 Helen Ginn
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

#include "Structure.h"
#include <iostream>

Structure::Structure(std::string filename) : SlipObjFile(filename)
{
	resize(1);
	setName("Structure");
	recolour(0, 0.5, 0.5);
	_triangulation = 0;
	
	bool collapsed = collapseCommonVertices();
	
	if (collapsed)
	{
		writeObjFile("smaller-" + filename);
	}
}

void Structure::clearExtra()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].extra[0] = 0;
	}

}

void Structure::markExtraAround(vec3 pos, double reach)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&v, pos);
		
		if (vec3_length(v) > reach)
		{
			continue;
		}
		
		_vertices[i].extra[0] = 1;
	}
}

void Structure::convertExtraToColour()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (_vertices[i].extra[0] < 0.5)
		{
			continue;
		}

		bool variagated = false;
		for (size_t j = 0; j < _indices.size(); j += 3)
		{
			if (_indices[j] == i || _indices[j+1] == i || _indices[j+2] == i)
			{
				if (_vertices[_indices[j+0]].extra[0] < 0.5 ||
				    _vertices[_indices[j+1]].extra[0] < 0.5 ||
				    _vertices[_indices[j+2]].extra[0] < 0.5)
				{
					variagated = true;
					break;
				}
			}
		}

		if (variagated)
		{
			_vertices[i].color[0] = 0;
			_vertices[i].color[1] = 0;
			_vertices[i].color[2] = 0;
		}
		else
		{
			if (_vertices[i].color[0] < 1e-6 &&
			    _vertices[i].color[1] < 1e-6 &&
			    _vertices[i].color[2] < 1e-6)
			{
				continue;
			}

			_vertices[i].color[0] = 1.0;
			_vertices[i].color[1] = 1.0;
			_vertices[i].color[2] = 1.0;
		}
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].extra[0] = 0;
	}
}

void Structure::triangulate()
{
	if (_triangulation >= 1)
	{
		return;
	}
	
	SlipObject::triangulate();

	_triangulation++;
}
