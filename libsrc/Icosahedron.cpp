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

#include "Icosahedron.h"
#include <iostream>

Icosahedron::Icosahedron()
{
	makeIco();
}

void Icosahedron::makeIco()
{
	addVertex(2, 1, 0); // 0
	addVertex(-2, -1, 0); // 1
	addVertex(-2, 1, 0); // 2
	addVertex(2, -1, 0); // 3

	addVertex(0, 2, 1); // 4
	addVertex(0, -2, -1); // 5
	addVertex(0, -2, 1); // 6
	addVertex(0, 2, -1); // 7

	addVertex(1, 0, 2); // 8
	addVertex(-1, 0, -2); // 9
	addVertex(1, 0, -2); // 10
	addVertex(-1, 0, 2); // 11
	
	addIndices(6, 11, 8);
	addIndices(6, 1, 11);
	addIndices(6, 5, 1);
	addIndices(1, 9, 5);
	addIndices(1, 11, 2);
	addIndices(1, 2, 9);
	addIndices(3, 6, 8);
	addIndices(3, 5, 6);
	addIndices(3, 10, 5);
	addIndices(3, 0, 10);
	addIndices(3, 8, 0);
	addIndices(2, 7, 9);
	addIndices(2, 4, 7);
	addIndices(2, 11, 4);
	addIndices(10, 0, 7);
	addIndices(10, 7, 9);
	addIndices(10, 9, 5);
	addIndices(4, 8, 11);
	addIndices(4, 0, 8);
	addIndices(4, 0, 7);
	
	resize(1/sqrt(5));
}

void Icosahedron::triangulate()
{
	vec3 centre = centroid();
	double ave = 0;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&v, centre);
		ave += vec3_length(v);
	}
	
	ave /= (double)_vertices.size();

	SlipObject::triangulate();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&v, centre);
		vec3_set_length(&v, ave);
		vec3_add_to_vec3(&v, centre);
		pos_from_vec(_vertices[i].pos, v);
	}
	
	calculateNormals();
}

void Icosahedron::removeUselessVertices(bool flaps)
{
	changeToTriangles();

	int count = 0;
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 vtx = vec_from_pos(_vertices[i].pos);
		std::vector<GLuint> neighbours;
		std::vector<GLuint *> places;
		std::map<GLuint, GLuint> pairs;

		for (size_t j = 0; j < _indices.size(); j += 3)
		{
			if (_indices[j] != i && _indices[j + 1] != i 
			    && _indices[j + 2] != i)
			{
				continue;
			}

			places.push_back(&_indices[j]);
		}
		
		double all_angles = 0;
		for (size_t j = 0; j < places.size(); j++)
		{
			bool swap = false;
			std::vector<GLuint> these;
			for (int k = 0; k < 3; k++)
			{
				if (places[j][k] == i)
				{
					if (k == 1) { swap = true; }
					continue;
				}

				these.push_back(places[j][k]);
				neighbours.push_back(places[j][k]);
			}
			
			if (swap)
			{
				GLuint tmp = these[0];
				these[0] = these[1];
				these[1] = tmp;
			}
			
			vec3 vtx2 = vec_from_pos(_vertices[these[0]].pos);
			vec3 vtx3 = vec_from_pos(_vertices[these[1]].pos);
			vec3_subtract_from_vec3(&vtx3, vtx);
			vec3_subtract_from_vec3(&vtx2, vtx);
			double angle = vec3_angle_with_vec3(vtx2, vtx3);
			all_angles += angle;

			pairs[these[0]] = these[1];
		}
		
		double offset = fabs(360 - (180/M_PI)*all_angles);
		
		if (pairs.size() == 1)
		{
			places[0][0] = 0;
			places[0][1] = 0;
			places[0][2] = 0;
		}

		if (offset > 4 || flaps)
		{
			continue;
		}

		if (pairs.size() == 3 && places.size() == 3)
		{
			count++;
			places[0][0] = neighbours[0];
			places[0][1] = pairs[places[0][0]];
			places[0][2] = pairs[places[0][1]];

			for (int j = 1; j <= 2; j++)
			{
				places[j][0] = 0;
				places[j][1] = 0;
				places[j][2] = 0;
			}
		}

		if (pairs.size() == 6 && places.size() == 6)
		{
			count++;
			places[0][0] = neighbours[0];
			places[0][1] = pairs[places[0][0]];
			places[0][2] = pairs[places[0][1]];

			places[1][0] = places[0][2];
			places[1][1] = pairs[places[1][0]];
			places[1][2] = pairs[places[1][1]];

			places[2][0] = places[1][2];
			places[2][1] = pairs[places[2][0]];
			places[2][2] = pairs[places[2][1]];

			places[3][0] = places[0][0];
			places[3][1] = places[1][0];
			places[3][2] = places[2][0];

			for (int j = 4; j <= 5; j++)
			{
				places[j][0] = 0;
				places[j][1] = 0;
				places[j][2] = 0;
			}
		}
	}
	
	std::vector<GLuint> is = _indices;
	_indices.clear();
	for (size_t i = 0; i < is.size(); i += 3)
	{
		if (is[i] != 0 || is[i+1] != 0 || is[i+2] != 0)
		{
			addIndices(is[i], is[i+1], is[i+2]);
		}
	}
	
	std::cout << count << " removals." << std::endl;
	changeToLines();
	
	if (!flaps)
	{
		removeUselessVertices(true);
	}
}

void Icosahedron::calculateNormals()
{
	vec3 centre = centroid();
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		vec3 pos1 = vec_from_pos(_vertices[_indices[i+0]].pos);
		vec3 pos2 = vec_from_pos(_vertices[_indices[i+1]].pos);
		vec3 pos3 = vec_from_pos(_vertices[_indices[i+2]].pos);

		vec3 diff31 = vec3_subtract_vec3(pos3, pos1);
		vec3 diff21 = vec3_subtract_vec3(pos2, pos1);

		vec3 cross = vec3_cross_vec3(diff31, diff21);
		vec3 diff = vec3_subtract_vec3(pos1, centre);
		
		double dot = vec3_dot_vec3(diff, cross);
		if (dot < 0)
		{
			GLuint tmp = _indices[i];
			_indices[i] = _indices[i+1];
			_indices[i+1] = tmp;
		}
	}

	SlipObject::calculateNormals();
}
