// Slip n Slide
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

#include <iostream>
#include "SlipPanel.h"
#include "SlipObject.h"

SlipPanel::SlipPanel(vec3 x1, vec3 x2, vec3 x3) : SlipObject()
{
	/* local panel copy */
	setupVertices(x1, x2, x3);
}

void SlipPanel::setupVertices(vec3 x1, vec3 x2, vec3 x3)
{
	_vertices.clear();
	_indices.clear();
	
	_indices.push_back(0);
	_indices.push_back(1);
	_indices.push_back(2);
	
	Vertex v;
	memset(v.pos, 0, sizeof(Vertex));

	v.color[3] = 1;
	v.pos[0] = x1.x;
	v.pos[1] = x1.y;
	v.pos[2] = x1.z;
	_vertices.push_back(v);
	
	v.pos[0] = x2.x;
	v.pos[1] = x2.y;
	v.pos[2] = x2.z;
	_vertices.push_back(v);

	v.pos[0] = x3.x;
	v.pos[1] = x3.y;
	v.pos[2] = x3.z;
	_vertices.push_back(v);
	
	std::cout << "Yes" << std::endl;
}

