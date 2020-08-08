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
#include <RefinementStrategy.h>
#include "Structure.h"
#include <Fibonacci.h>
#include "Data.h"

double Bound::_radius = 14.;
bool Bound::_updateOnRender = false;

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
	_nearestNorm = make_vec3(1, 0, 0);
	_special = false;
	_snapping = false;
	_fixed = false;
	setColour(0.8, 0.8, 0.8);
	triangulate();
	resize(2);
	_central = true;

	setSelectable(true);
}

void Bound::snapToObject(SlipObject *obj)
{
	if (obj == NULL)
	{
		obj = _structure;
	}
	
	vec3 p = centroid();
	vec3 nearest = obj->nearestVertex(p, true);

	vec3 diff = vec3_subtract_vec3(nearest, p);
	addToVertices(diff);
	_realPosition = centroid();
}

void Bound::randomlyPositionInRegion(SlipObject *obj)
{
	if (_fixed)
	{
		return;
	}
	
	/*
	vec3 min, max;
	obj->boundaries(&min, &max);
	vec3 scale = vec3_subtract_vec3(max, min);
	vec3 place = random_vec3(true);
	place.x *= scale.x;
	place.y *= scale.y;
	place.z *= scale.z;
	vec3_add_to_vec3(&place, min);
	*/
	vec3 place = obj->randomVertex();

	vec3 myCentroid = centroid();
	vec3 diff = vec3_subtract_vec3(place, myCentroid);

	addToVertices(diff);
}

void Bound::jiggleOnSurface(SlipObject *obj)
{
	if (_fixed)
	{
		return;
	}
	
	vec3 jiggle = random_vec3();
	addToVertices(jiggle);
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
		if (_updateOnRender)
		{
			std::lock_guard<std::mutex> l(_mutex);
			nearest = _structure->nearestVertex(_realPosition);
			return nearest;
		}

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

	if (_structure->hasMesh())
	{
		Vertex *close = _structure->nearestVertexPtr(working, true);
		_nearestNorm = vec_from_pos(close->normal);
	}

	vec3 diff = vec3_subtract_vec3(working, current);
	lockMutex();
	addToVertices(diff);
	unlockMutex();

}

void Bound::render(SlipGL *gl)
{
	SlipObject::render(gl);
}

void Bound::addToStrategy(RefinementStrategy *str)
{
	if (_fixed)
	{
		return;
	}

	double step = 10;
	double tol = 0.01;
	str->addParameter(this, Bound::getPosX, Bound::setPosX,
	                  (rand() % 2 - 0.5) * step, tol, name() + "_x");
	str->addParameter(this, Bound::getPosY, Bound::setPosY,
	                  (rand() % 2 - 0.5) * step, tol, name() + "_y");
	str->addParameter(this, Bound::getPosZ, Bound::setPosZ,
	                  (rand() % 2 - 0.5) * step, tol, name() + "_z");
}

void Bound::setRealPosition(vec3 real)
{
	_realPosition = real;

	vec3 current = centroid();
	vec3 diff = vec3_subtract_vec3(real, current);
	addToVertices(diff);
}

double Bound::carefulScoreWithOther(Bound *other)
{
	if (!_structure->hasMesh())
	{
		return 1;
	}

	vec3 mynorm = _nearestNorm;
	vec3 yournorm = other->_nearestNorm;

	double dot = vec3_dot_vec3(mynorm, yournorm);
	dot /= 4;
	dot += 0.75;
	
	return dot;
}

double Bound::scoreWithOther(Bound *other)
{
	std::string bin = name();
	vec3 posi = getWorkingPosition();
	std::string bjn = other->name();
	
	vec3 posj = other->getWorkingPosition();

	vec3 vector = vec3_subtract_vec3(posi, posj);
	double distance = vec3_length(vector);
	double radius = getRadius();

	double prop = 0;
	if (distance < 2 * radius)
	{
		double q = (2 * radius - distance) / 2;
		q /= 2 * radius;
		prop = 2 * (3 * q * q - 2 * q * q * q);
	}

	prop = pow(prop, 0.35);
	double slide = std::max(0.2 * (1 - distance / 100.), 0.);
	prop += slide;
	prop = std::min(1., prop);
	
	/*
	double decrease = carefulScoreWithOther(other);
	prop *= decrease;
	*/

	return prop;
}

double Bound::percentageCloudInOther(Bound *b)
{
	filterCloud();
	vec3 other = b->getWorkingPosition();
	double sqr = getRadius();
	double clash = 0;

	for (size_t i = 0; i < _viableCloud.size(); i++)
	{
		vec3 p = _viableCloud[i];
		vec3_subtract_from_vec3(&p, other);
		
		double sql = vec3_sqlength(p);
		if (sql < sqr)
		{
			clash++;
		}
	}

	return clash / (double)_viableCloud.size();
}

void Bound::filterCloud()
{
	_viableCloud.clear();
	if (_pointCloud.size() == 0)
	{
		cloud(120);
	}

	vec3 centre = centroid();
	for (size_t i = 0; i < _pointCloud.size(); i++)
	{
		vec3 p = _pointCloud[i];
		vec3_add_to_vec3(&p, centre);

		if (!_structure->pointInside(p))
		{
			_viableCloud.push_back(p);
		}
	}
}

void Bound::cloud(double totalPoints)
{
	double totalSurfaces = 0;
	double factor = pow(totalPoints, 1./3.) * 2;
	int layers = lrint(factor);
	_pointCloud.clear();

	layers = 1;
	
	std::vector<double> layerSurfaces;

	/* Work out relative ratios of the surfaces on which points
	 * will be generated. */
	for (int i = 1; i <= layers; i++)
	{
		layerSurfaces.push_back(i * i);
		totalSurfaces += i * i;
	}

	double scale = totalPoints / (double)totalSurfaces;

	double addTotal = 0;
	Fibonacci fib;
	fib.generateLattice(layers, 1);
	std::vector<vec3> directions = fib.getPoints();
	
	if (totalPoints < 2)
	{
		_pointCloud.push_back(empty_vec3());
	}

	_pointCloud.reserve(totalPoints);
	vec3 yAxis = make_vec3(0, 1, 0);

	for (int j = 0; j < layers; j++)
	{
		vec3 cross = vec3_cross_vec3(directions[j], yAxis);
		
		mat3x3 mat = mat3x3_closest_rot_mat(yAxis, directions[j], cross);

		double m = getRadius() * (double)(j + 1) / (double)layers;

		int samples = layerSurfaces[j] * scale + 1;
		
		fib.generateLattice(samples, m);
		std::vector<vec3> points = fib.getPoints();
		
		for (size_t i = 0; i < points.size(); i++)
		{
			double add = 1;
			addTotal += add;
			
			mat3x3_mult_vec(mat, &points[i]);

			_pointCloud.push_back(points[i]);
		}
	}
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

