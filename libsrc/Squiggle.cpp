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

#include "Squiggle.h"
#include "Result.h"
#include "shaders/White.h"
#include <iostream>

Squiggle::Squiggle() : SlipObject()
{
	_renderType = GL_LINES;
	_vString = White_vsh();

}

void Squiggle::clear()
{
	_vertices.clear();
	_indices.clear();
	_results.clear();
}

void Squiggle::setPositions(std::vector<vec3> poz,
                            std::vector<Result *> results)
{
	_vertices.clear();
	_indices.clear();
	_results.clear();
	
	if (poz.size() == 0)
	{
		return;
	}

	vec3 p = poz[0];
	addVertex(p.x, p.y, p.z);
	_results.push_back(results[0]);

	for (size_t i = 1; i < poz.size(); i++)
	{
		vec3 p = poz[i];
		addVertex(p.x, p.y, p.z);
		_results.push_back(results[i]);
		addIndex(i-1);
		addIndex(i);
	}

	recolour(1, 1, 1);
}

std::vector<Result *> Squiggle::findResultsBetween(double x1, double y1,
                                                   double x2, double y2)
{
	std::vector<Result *> chosen;

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 p = vec_from_pos(_vertices[i].pos);
		double last = 1;
		vec3 model = mat4x4_mult_vec3(_model, p, &last);
		vec3 proj = mat4x4_mult_vec3(_proj, model, &last);
		vec3_mult(&proj, 1 / last);
		std::cout << vec3_desc(proj) << std::endl;
		
		if (proj.x > x1 && proj.x < x2 && proj.y > y2 && proj.y < y1)
		{
			chosen.push_back(_results[i]);
		}
	}

	std::cout << "Found " << chosen.size() << std::endl;
	return chosen;
}
