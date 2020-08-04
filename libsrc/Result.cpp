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

#include "Result.h"
#include "Experiment.h"
#include "Bound.h"

Result::Result()
{
	_score = 0;

}

void Result::applyPositions(Experiment *exp)
{
	for (size_t i = 0; i < exp->boundCount(); i++)
	{
		Bound *bi = exp->bound(i);
		if (_map.count(bi) == 0)
		{
			continue;
		}

		vec3 wip = _map[bi];
		bi->setRealPosition(wip);
		bi->updatePositionToReal();
	}
}

void Result::savePositions(Experiment *exp)
{
	_map.clear();

	for (size_t i = 0; i < exp->boundCount(); i++)
	{
		vec3 wip = exp->bound(i)->getWorkingPosition();
		_map[exp->bound(i)] = wip;
	}

}

vec3 Result::vecForBound(Bound *b)
{
	return _map[b];
}
