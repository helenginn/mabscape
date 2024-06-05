// vagabond
// Copyright (C) 2022 Helen Ginn
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

#ifndef __vagabond__GhostBound__
#define __vagabond__GhostBound__

#include "Bound.h"

class GhostBound : public Bound
{
public:
	GhostBound(Bound *copy, mat4x4 mat) : Bound()
	{
		_copy = copy;
		_sym = mat;
		setColour(0.3, 0.3, 0.3);
		_realPosition = getWorkingPosition();
		updatePositionToReal();
	}

	void randomlyPositionInRegion(SlipObject *obj)
	{
		vec3 place = getWorkingPosition();

		vec3 myCentroid = centroid();
		vec3 diff = vec3_subtract_vec3(place, myCentroid);

		lockMutex();
		addToVertices(diff);
		unlockMutex();
		setRealPosition(place);
	}

	bool refineable()
	{
		return false;
	}

	vec3 getWorkingPosition()
	{
		vec3 wp = _copy->getWorkingPosition();
		return mat4x4_mult_vec(_sym, wp);
	}
	vec3 getStoredPosition()
	{
		return getWorkingPosition();
	}
private:
	Bound *_copy = nullptr;
	mat4x4 _sym;

};

#endif
