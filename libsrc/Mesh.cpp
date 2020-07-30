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

void Mesh::shrinkWrap()
{
	double speed = 0.1;
	std::vector<Vertex> vcopy = _vertices;

	for (int i = 0; i < 100; i++)
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
				vec3_mult(&diff, speed);
			}
			vec3_add_to_vec3(&vtx, diff);
			pos_from_vec(vcopy[i].pos, vtx);
		}
		
		double ave = averageRadius();

		for (size_t i = 0; i < vcopy.size(); i++)
		{
			vec3 vtx = vec_from_pos(vcopy[i].pos);
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
			vec3_mult(&booster, speed * 2);
			vec3_set_length(&norm, speed * 2);

			vec3_add_to_vec3(&vtx, booster);
			vec3_add_to_vec3(&vtx, norm);

			pos_from_vec(vcopy[i].pos, vtx);
		}
		
		double now = averageRadius();
		resize(1.0 * ave / now);
		calculateNormals();
		
		lockMutex();
		_vertices = vcopy;
		
		if (i % 100 == 0 && i > 0)
		{
//			removeUselessVertices();
		}
		unlockMutex();
	}
	
//	_parent->colourOutlayBlack();
	resultReady();
}
