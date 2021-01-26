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
#include "Arrow.h"
#include "Bound.h"
#include <helen3d/RefinementStrategy.h>
#include "Structure.h"
#include <libsrc/Fibonacci.h>
#include "Data.h"
#include <libsrc/shaders/vStructure.h>
#include <libsrc/shaders/fStructure.h>

using namespace Helen3D;

double Bound::_radius = 13.;
double Bound::_shoulderAngle = deg2rad(45.);

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

Bound::Bound(std::string filename) : Icosahedron()
{
	_elbowAngle = NAN;
	_arrow = NULL;
	_value = NAN;
	_nearestNorm = make_vec3(1, 0, 0);
	_special = false;
	_snapping = false;
	_fixed = false;
	setColour(0.7, 0.7, 0.7);
	triangulate();
	resize(2);
	_central = true;
	_fString = Structure_fsh();
	_vString = Structure_vsh();

	setSelectable(true);
}

double Bound::snapToObject(Structure *obj)
{
	if (obj == NULL)
	{
		obj = _structure;
	}
	
	vec3 p = centroid();
	vec3 nearest = obj->lookupVertexPtr(p);

	vec3 diff = vec3_subtract_vec3(nearest, p);
	double l = vec3_length(diff);
	lockMutex();
	addToVertices(diff);
	unlockMutex();
	_realPosition = centroid();
	
	return l;
}

void Bound::randomlyPositionInRegion(SlipObject *obj)
{
	if (_fixed)
	{
		return;
	}
	
	vec3 place = obj->randomVertex();

	vec3 myCentroid = centroid();
	vec3 diff = vec3_subtract_vec3(place, myCentroid);

	lockMutex();
	addToVertices(diff);
	unlockMutex();
	_realPosition = place;
}

void Bound::jiggleOnSurface(Structure *obj)
{
	if (_fixed)
	{
		return;
	}
	
	vec3 jiggle = random_vec3();
	lockMutex();
	addToVertices(jiggle);
	unlockMutex();
	snapToObject(obj);
}

void Bound::colourFixed()
{
	if (_fixed)
	{
		recolour(0.0, 0, 0, &_unselectedVertices);
		recolour(0.0, 0, 0, &_vertices);
	}
	else
	{
		recolour(_red, _green, _blue, &_unselectedVertices);
		recolour(_red, _green, _blue, &_vertices);
	}
}

void Bound::toggleFixPosition()
{
	_fixed = !_fixed;
	colourFixed();
}

vec3 Bound::getWorkingPosition()
{
	if (!_snapping)
	{
		return _realPosition;
	}
	else
	{
		vec3 nearest;
		nearest = _structure->nearestVertex(_realPosition);
		return nearest;
	}
}


void Bound::setSnapping(bool snapping)
{
	_snapping = snapping;
	_realPosition = centroid();
}

void Bound::updatePositionToReal()
{
	vec3 current = centroid();
	vec3 working = getWorkingPosition();

	vec3 diff = vec3_subtract_vec3(working, current);
	lockMutex();
	addToVertices(diff);
	unlockMutex();

	if (_arrow != NULL)
	{
		redrawElbow();
		_arrow->populate();
	}
}

void Bound::redrawElbow()
{
	if (_arrow == NULL || _elbowAngle != _elbowAngle)
	{
		return;
	}

	vec3 norm = _structure->lookupVertexPtr(_realPosition, true);
	vec3 elbow = norm;
	
	vec3 zAxis = make_vec3(0, 0, 1);
	vec3 cross = vec3_cross_vec3(zAxis, norm);
	vec3_set_length(&cross, 1);

	mat3x3 rot = mat3x3_unit_vec_rotation(cross, _shoulderAngle);
	mat3x3_mult_vec(rot, &elbow);
	
	mat3x3 swing = mat3x3_unit_vec_rotation(norm, _elbowAngle);
	mat3x3_mult_vec(swing, &elbow);

	vec3_mult(&elbow, 4.);
	vec3 start = vec3_add_vec3(_realPosition, elbow);
	vec3_mult(&elbow, 6.);
	vec3 end = vec3_add_vec3(start, elbow);
	
	_arrow->setEnds(start, end);
}

void Bound::enableElbow()
{
	randomiseElbow();
	_arrow = new Arrow(this);
	_arrow->setSelectable(true);
	redrawElbow();
	_arrow->populate();
}

void Bound::render(SlipGL *gl)
{
	SlipObject::render(gl);
	
	if (_arrow != NULL)
	{
		redrawElbow();
		_arrow->render(gl);
	}
}

void Bound::addToStrategy(RefinementStrategy *str, bool elbow)
{
	if (_fixed)
	{
		return;
	}

	double step = 10;
	double tol = 0.01;
	if (!elbow)
	{
		str->addParameter(this, Bound::getPosX, Bound::setPosX,
		                  (rand() % 2 - 0.5) * step, tol, name() + "_x");
		str->addParameter(this, Bound::getPosY, Bound::setPosY,
		                  (rand() % 2 - 0.5) * step, tol, name() + "_y");
		str->addParameter(this, Bound::getPosZ, Bound::setPosZ,
		                  (rand() % 2 - 0.5) * step, tol, name() + "_z");
	}
	
	if (elbow && _arrow != NULL)
	{
		str->addParameter(this, Bound::getElbowAngle, Bound::setElbowAngle,
		                  deg2rad(90.), deg2rad(2.), name() + "elbow");
	}
}

void Bound::setRealPosition(vec3 real)
{
	_realPosition = real;

	vec3 current = centroid();
	vec3 diff = vec3_subtract_vec3(real, current);
	lockMutex();
	addToVertices(diff);
	unlockMutex();
}

double Bound::sigmoidalScore(vec3 posi, vec3 posj, double slope, double mult)
{
	vec3 vector = vec3_subtract_vec3(posi, posj);
	double x = vec3_length(vector);

	vec3 mid = vec3_add_vec3(posi, posj);
	vec3_mult(&mid, 0.5);

	double inflection = _radius * 2 * mult;
	double exponent = exp((inflection - x) / slope);
	double val = exponent / (1 + exponent);
	
	return val;

}

double Bound::scoreWithOther(Bound *other, bool dampen)
{
	vec3 posi = getStoredPosition();
	vec3 posj = other->getStoredPosition();

	double val = sigmoidalScore(posi, posj);

	if (_arrow != NULL && other->_arrow != NULL)
	{
		redrawElbow();
		vec3 endi = _arrow->end();
		vec3 endj = other->_arrow->end();

		val += sigmoidalScore(endi, endj, 10.0, 2.0) * (1 - val);
	}
	
	return val;
}

void Bound::setSpecial(bool special)
{
	_special = special;

	if (_special)
	{
		setColour(0.8, 0.8, 0.0);
	}
	else
	{
		setColour(0.8, 0.8, 0.8);
	}
}

void Bound::colourByValue(double stdev)
{
	double val = _value / stdev;
	double red = 0;
	double green = 0;
	double blue = 0;

	if (val != val) /* we go grey */
	{
		red = 100;
		green = 100;
		blue = 100;
	}
	else if (val < 0.0) /* we go blue. */
	{
		val = -val;
		
		red = 255 - val * 255;
		green = 255 - val * 255;
		blue = 255;
	}
	else if (val >= 0.0) /* We go red. */
	{
		red = 255;
		green = 255 - val * 255;
		blue = 255 - val * 255;
	}

	red /= 255; green /= 255; blue /= 255;
	red *= 0.8; green *= 0.8; blue *= 0.8;
	setColour(red, green, blue);
}

void Bound::randomiseElbow()
{
	_elbowAngle = deg2rad((double)(rand() % 360));
}

void Bound::selectMe(bool sel)
{
	setSelected(sel);
	
	if (_arrow)
	{
		_arrow->setSelected(sel);
	}
}
