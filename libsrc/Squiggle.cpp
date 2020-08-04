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
#include "shaders/White.h"

Squiggle::Squiggle() : SlipObject()
{
	_renderType = GL_LINES;
	_vString = White_vsh();

}

void Squiggle::clear()
{
	_vertices.clear();
	_indices.clear();
}

void Squiggle::setPositions(std::vector<vec3> poz)
{
	_vertices.clear();
	_indices.clear();

	vec3 p = poz[0];
	addVertex(p.x, p.y, p.z);

	for (size_t i = 1; i < poz.size(); i++)
	{
		vec3 p = poz[i];
		addVertex(p.x, p.y, p.z);
		addIndex(i-1);
		addIndex(i);
	}

	recolour(1, 1, 1);
}

