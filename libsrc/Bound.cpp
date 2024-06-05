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
#include <hcsrc/RefinementStrategy.h>
#include <h3dsrc/Text.h>
#include "Structure.h"
#include <hcsrc/Fibonacci.h>
#include "Data.h"
#include <h3dsrc/shaders/vStructure.h>
#include <h3dsrc/shaders/fStructure.h>

using namespace Helen3D;

double Bound::_radius = 11.;
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

Bound::Bound() : Icosahedron()
{
	_text = NULL;
	_rmsd = 0;
	_elbowAngle = NAN;
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
	
	vec3 p = getStoredPosition();
	vec3 nearest = obj->lookupVertexPtr(p);
	vec3 diff = vec3_subtract_vec3(nearest, p);
	double l = vec3_length(diff);

	setRealPosition(nearest);
	updatePositionToReal();
	
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
	setRealPosition(place);
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
		return getStoredPosition();
	}
	else
	{
		vec3 nearest;
		nearest = _structure->nearestVertex(getStoredPosition());
		return nearest;
	}
}


void Bound::setSnapping(bool snapping)
{
	_snapping = snapping;
	setRealPosition(centroid());
}

void Bound::updatePositionToReal()
{
	vec3 current = centroid();
	vec3 diff = vec3_subtract_vec3(_realPosition, current);
	lockMutex();
	addToVertices(diff);
	unlockMutex();
}

void Bound::render(SlipGL *gl)
{
	SlipObject::render(gl);
	
	if (_text != NULL)
	{
		_text->render(gl);
	}
}

void Bound::addToStrategy(RefinementStrategy *str, bool elbow)
{
	if (_fixed)
	{
		return;
	}

	double step = 10;
	double tol = 0.1;
	if (!elbow)
	{
		str->addParameter(this, Bound::getPosX, Bound::setPosX,
		                  (rand() % 2 - 0.5) * step, tol, name() + "_x");
		str->addParameter(this, Bound::getPosY, Bound::setPosY,
		                  (rand() % 2 - 0.5) * step, tol, name() + "_y");
		str->addParameter(this, Bound::getPosZ, Bound::setPosZ,
		                  (rand() % 2 - 0.5) * step, tol, name() + "_z");
	}
}

void Bound::setRealPosition(vec3 real)
{
	if (real.x != real.x || real.y != real.y || real.z != real.z)
	{
		return;
	}

	_realPosition = real;
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

void Bound::findNearestNorm()
{
	vec3 pos = getStoredPosition();
	_nearestNorm = _structure->lookupVertexPtr(pos, true);
}

double Bound::scoreWithOther(Bound *other, bool dampen)
{
	vec3 posi = getStoredPosition();
	vec3 posj = other->getStoredPosition();
	
	double val = sigmoidalScore(posi, posj);

	double dot = vec3_dot_vec3(_nearestNorm, 
	                           other->_nearestNorm);

	if (dot < 0)
	{
		dot += 1;
		val *= dot;
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

void Bound::colourByValue(double mean, double stdev)
{
	double val = (_value - mean) / stdev;
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
}

void Bound::label(bool visible)
{
	if (_text != NULL)
	{
		delete _text;
		_text = NULL;
	}

	if (!visible)
	{
		return;
	}
	
	_text = new Text();
	_text->setProperties(getStoredPosition(), name(), 144, Qt::black,
	                     0, 0, 5);
	_text->prepare();
}

void Bound::radiusOnStructure(Structure *str, double rad)
{
	int count = 0;
	for (size_t i = 0; i < str->vertexCount(); i++)
	{
		vec3 pos = vec_from_pos(str->vertex(i).pos);
		vec3 diff = getStoredPosition() - pos;
		
		double length = vec3_length(diff);
		
		if (length < rad)
		{
			str->vPointer()[i].tex[0] += 1;
			count++;
		}
	}

	setDisabled(true);
	std::cout << name() << " affected " << count << " vertices." << std::endl;
}
