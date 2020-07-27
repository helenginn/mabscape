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

#include <iostream>
#include "Bound.h"

vec3 random_vec3(bool absolute = false)
{
	vec3 jiggle = make_vec3(0, 0, 0);
	jiggle.x = rand() / (double)RAND_MAX - 0.5;
	jiggle.y = rand() / (double)RAND_MAX - 0.5;
	jiggle.z = rand() / (double)RAND_MAX - 0.5;
	vec3_mult(&jiggle, 2);
	
	if (absolute)
	{
		jiggle.x = fabs(jiggle.x);
		jiggle.y = fabs(jiggle.y);
		jiggle.z = fabs(jiggle.z);
	}

	return jiggle;
}

Bound::Bound(std::string filename) : SlipObjFile(filename)
{
	_fixed = false;
	recolour(1, 0, 0);
	resize(1);
	_central = true;

	setSelectable(true);
}

void Bound::snapToObject(SlipObject *obj)
{
	vec3 myCentroid = centroid();
	vec3 nearest = obj->nearestVertex(myCentroid);
	vec3 diff = vec3_subtract_vec3(nearest, myCentroid);
	addToVertices(diff);
}

void Bound::randomlyPositionInRegion(SlipObject *obj)
{
	vec3 min, max;
	obj->boundaries(&min, &max);
	vec3 scale = vec3_subtract_vec3(max, min);
	vec3 place = random_vec3(true);
	place.x *= scale.x;
	place.y *= scale.y;
	place.z *= scale.z;
	vec3_add_to_vec3(&place, min);

	vec3 myCentroid = centroid();
	vec3 diff = vec3_subtract_vec3(place, myCentroid);

	addToVertices(diff);
}

void Bound::jiggleOnSurface(SlipObject *obj)
{
	vec3 jiggle = random_vec3();
	addToVertices(jiggle);
	snapToObject(obj);
}

void Bound::toggleFixPosition()
{
	_fixed = !_fixed;

	if (_fixed)
	{
		recolour(0.0, 0, 0, &_unselectedVertices);
		recolour(0.0, 0, 0, &_vertices);
	}
	else
	{
		recolour(1, 0, 0, &_unselectedVertices);
		recolour(0, 0, 1, &_vertices);
	}
}
