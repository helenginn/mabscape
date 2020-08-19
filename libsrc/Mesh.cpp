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

#include "Mesh.h"
#include <iostream>

double Mesh::_speed = 0.2;

Mesh::Mesh(SlipObject *other) : Icosahedron()
{
	double rad = other->envelopeRadius();
	fixCentroid(other->centroid());
	resize(rad * 1.2);
	recolour(1, 1, 1);
	triangulate();
	triangulate();
	triangulate();
	std::cout << "Mesh with " << _indices.size() / 3 << " faces." << std::endl;
	changeToLines();
	_parent = other;
}

void Mesh::hug(std::vector<Vertex> &vcopy)
{
	for (size_t i = 0; i < vcopy.size(); i++)
	{
		bool isBehind = false;
		vec3 vtx = vec_from_pos(vcopy[i].pos);
		vec3 norm = vec_from_pos(vcopy[i].normal);
		vec3 nearest = _parent->nearestVertexNearNormal(vtx, norm, 
		                                                &isBehind);
		vec3 diff = vec3_subtract_vec3(nearest, vtx);
		if (!isBehind)
		{
			vec3_mult(&diff, _speed);
		}
		vec3_add_to_vec3(&vtx, diff);
		pos_from_vec(vcopy[i].pos, vtx);
	}
}

void Mesh::smoothen(std::vector<Vertex> &vcopy)
{
	vec3 centre = centroid();
	double all = 0;

	for (size_t i = 0; i < vcopy.size(); i++)
	{
		vec3 vtx = vec_from_pos(vcopy[i].pos);
		vec3 chk = vec3_subtract_vec3(vtx, centre);
		double length = vec3_length(chk);
		all += length;
		double longest = -1;
		vec3 target = empty_vec3();
		vec3 local_ctr = empty_vec3();
		double count = 0;

		for (size_t j = 0; j < _indices.size(); j += 2) /* GL LINES */
		{
			if (_indices[j] != i && _indices[j + 1] != i)
			{
				continue;
			}

			int other = (_indices[j] == i) ? _indices[j + 1] : _indices[j];
			vec3 pos = vec_from_pos(vcopy[other].pos);
			vec3_add_to_vec3(&local_ctr, pos);

			vec3_subtract_from_vec3(&pos, vtx);
			double length = vec3_sqlength(pos);

			count++;

			if (longest < length)
			{
				longest = length;
				target = pos;
			}
		}

		if (longest < 0)
		{
			continue;
		}

		vec3 norm = vec_from_pos(vcopy[i].normal);
		vec3_mult(&local_ctr, 1 / count);

		vec3 booster = vec3_subtract_vec3(local_ctr, vtx);
		vec3_mult(&booster, _speed * 1.25);
		vec3_set_length(&norm, _speed * 1.25);

		vec3_add_to_vec3(&vtx, booster);
		vec3_add_to_vec3(&vtx, norm);

		pos_from_vec(vcopy[i].pos, vtx);
	}

	all /= vcopy.size();
}

void Mesh::shrinkWrap()
{
	std::vector<Vertex> vcopy = _vertices;

	for (int i = 0; i < 100; i++)
	{
		hug(vcopy);
		smoothen(vcopy);
		
		lockMutex();
		_vertices = vcopy;
		calculateNormals();
		unlockMutex();
	}
	
	resultReady();
}

void Mesh::smoothCycles()
{
	std::vector<Vertex> vcopy = _vertices;

	for (int i = 0; i < 15; i++)
	{
		double ave = averageRadius();
		smoothen(vcopy);

		lockMutex();
		_vertices = vcopy;
		unlockMutex();

		double now = averageRadius();
		resize(1.0 * ave / now);
		calculateNormals();
	}

	resultReady();
}

void Mesh::inflateCycles()
{
	std::vector<Vertex> vcopy = _vertices;

	for (int i = 0; i < 15; i++)
	{
		lockMutex();
		resize(1.01);
		unlockMutex();
	}

	resultReady();
}
