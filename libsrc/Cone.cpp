// mabscape
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

#include "Cone.h"
#include "Glowable_sh.h"

Cone::Cone(vec3 axis, double height) : SlipObject()
{
	_vString = Glowable_vsh();
	_fString = Glowable_fsh();
	vec3_set_length(&axis, -1);
	std::vector<vec3> circle;

	const int divisions = 10;
	vec3 xAxis = make_vec3(1, 0, 0);

	double angle = (2 * M_PI) / (double)divisions;
	mat3x3 rot = mat3x3_unit_vec_rotation(axis, angle);
	vec3 cross = vec3_cross_vec3(axis, xAxis);
	vec3_set_length(&cross, 1.0);

	for (size_t i = 0; i < divisions; i++)
	{
		circle.push_back(cross);
		mat3x3_mult_vec(rot, &cross);
	}
	
	addCircle(empty_vec3(), circle);
	
	for (size_t i = 0; i < divisions; i++)
	{
		vec3 pos = axis * height;
		vec3 normal = axis;
		addVertex(pos.x, pos.y, pos.z);
		_vertices.back().normal[0] = normal.x;
		_vertices.back().normal[1] = normal.y;
		_vertices.back().normal[2] = normal.z;
	}

	addCylinderIndices(divisions);
	recolour(1.5, 1.1, 0.5);
}

void Cone::addCircle(vec3 centre, std::vector<vec3> &circle)
{
	for (size_t i = 0; i < circle.size(); i++)
	{
		vec3 pos = vec3_add_vec3(centre, circle[i]);
		vec3 normal = pos;
		vec3_set_length(&normal, 1);
		addVertex(pos.x, pos.y, pos.z);
		_vertices.back().normal[0] = normal.x;
		_vertices.back().normal[1] = normal.y;
		_vertices.back().normal[2] = normal.z;
	}
}

void Cone::addCylinderIndices(size_t num)
{
	int begin = - num * 2;
	int half = num;

	for (size_t i = 0; i < num - 1; i++)
	{
		addIndex(begin + 0);
		addIndex(begin + half);
		addIndex(begin + 1);
		addIndex(begin + 1);
		addIndex(begin + half);
		addIndex(begin + half + 1);
		begin++;

	}

	int one = num;
	half -= num * 2 - 1;
	addIndex(begin + 0);
	addIndex(begin + half);
	addIndex(begin + one);
	addIndex(begin + half);
	addIndex(begin + one);
	addIndex(begin + 1);
}
