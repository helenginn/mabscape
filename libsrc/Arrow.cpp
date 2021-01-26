// breathalyser
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

#include "shaders/gArrow.h"
#include "shaders/vArrow.h"
#include "shaders/fArrow.h"
#include "Arrow.h"
#include "Bound.h"

Arrow::Arrow(Bound *parent) : SlipObject()
{
	_bound = parent;
	_renderType = GL_LINES;
	_gString = Arrow_gsh();
	_fString = Arrow_fsh();
	_vString = Arrow_vsh();
	this->SlipObject::setName("Arrow");
	
	_indices.push_back(0);
	_indices.push_back(1);
}

void Arrow::populate()
{
	_vertices.clear();

	Helen3D::Vertex v;
	memset(&v, '\0', sizeof(Helen3D::Vertex));
	v.color[0] = 216. / 255;
	v.color[1] = 185. / 255;
	v.color[2] = 82. / 255;
	v.color[3] = 1.;

	pos_from_vec(&v.pos[0], _start);

	lockMutex();
	_vertices.push_back(v);
	pos_from_vec(&v.pos[0], _end);
	_vertices.push_back(v);
	unlockMutex();
	setSelectable(true);
}


