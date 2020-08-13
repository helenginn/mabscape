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

#include "KFirst.h"
#include <RefinementStrategy.h>
#include <Any.h>

KFirst::KFirst()
{
	_first = NULL;
	_hideSecond = true;
	_correctX = false;
	_beginVal = -FLT_MAX;
}

void KFirst::refineCascade()
{
	KCorrectExp::refine();
	KOffset::refine();
	curveChinUp();
	curve()->copyYs(this);
	_lockOff = true;
	_kOff = _first->getKOff();
	_kOn = _first->getKOn();
	KLigOnOff::refine();
	_hideSecond = false;
	KFirst::populateYs();
	subtractOff();
}

void KFirst::subtractOff()
{
	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);
	
	std::vector<double> ys = curve()->ys();
	std::vector<double> backupys = _ys;
	
	for (size_t i = 0; i < ys.size(); i++)
	{
		if (_xs[i] > min[1] && _xs[i] < max[1])
		{
			ys[i] = _topLigand + ys[i] - _ys[i];
		}
	}
	
	_ys = ys;
	curve()->copyYs(this);
	_ys = backupys;

}

void KFirst::addToStrategy(RefinementStrategyPtr str)
{

}

void KFirst::refine()
{

}

void KFirst::populateYs()
{
	KLigOnOff::populateYs();

	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[1] && _xs[i] < max[1] && _hideSecond)
		{
			_ys[i] = nan(" ");
		}
	}
}
