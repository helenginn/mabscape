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

#include "KOffset.h"
#include <iostream>
#include <RefinementStrategy.h>
#include <RefinementNelderMead.h>
#include <Any.h>

double KOffset::_checkTime = 3;

KOffset::KOffset()
{
	_offset = 0;

}

void KOffset::refineCascade()
{
	KOffset::refine();
}

void KOffset::refine()
{
	if (!check())
	{
		return;
	}

	_anys.clear();
	double *boundary = curve()->regionPtr(1, true);
	AnyPtr any = AnyPtr(new Any(boundary));

	NelderMeadPtr mead = NelderMeadPtr(new RefinementNelderMead());
	mead->addParameter(&*any, Any::get, Any::set, 3.0, 0.001);
	mead->setEvaluationFunction(KOffset::getLateralScore, this);
	mead->setCycles(20);
	mead->refine();
	
	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	if (_correctX)
	{
		curve()->setRegion(0, min[0], min[1]);
	}

	mead = NelderMeadPtr(new RefinementNelderMead());
	KOffset::addToStrategy(mead);
	mead->setEvaluationFunction(KOffset::getScore, this);
	mead->setCycles(50);
	mead->refine();
}

bool KOffset::check()
{
	bool set = curve()->isRegionSet(0);
	if (!set)
	{
		std::cout << "Region 1 in curve not set." << std::endl;
		return false;
	}

	set = curve()->isRegionSet(1);
	if (!set)
	{
		std::cout << "Region 2 in curve not set." << std::endl;
		return false;
	}

	return true;
}

void KOffset::addToStrategy(RefinementStrategyPtr str)
{
	AnyPtr any = AnyPtr(new Any(&_offset));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.1, 0.001);
}

void KOffset::populateYs()
{
	double min; double max;
	curve()->getRegion(1, &min, &max);
	
	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min && _xs[i] < max)
		{
			_ys[i] -= _offset;
		}
	}
}

double KOffset::lateralScore()
{
	_ys = curve()->ys();
	double min[2], max[2];
	double total[2] = {0, 0};
	double count[2] = {0, 0};
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[1] - _checkTime && _xs[i] < min[1])
		{
			total[0] += _ys[i];
			count[0]++;
		}
		if (_xs[i] < min[1] + _checkTime && _xs[i] > min[1])
		{
			total[1] += _ys[i];
			count[1]++;
		}
	}
	
	total[0] /= count[0];
	total[1] /= count[1];
	
	return -fabs(total[1] - total[0]);
}

double KOffset::score()
{
	_mut.lock();
	_ys = curve()->ys();
	_mut.unlock();

	KOffset::populateYs();
	
	double min[2] = {0, 0};
	double max[2] = {0, 0};
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);
	
	double total[2] = {0, 0};
	double count[2] = {0, 0};
	
	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > max[0] - _checkTime && _xs[i] < max[0])
		{
			total[0] += _ys[i];
			count[0]++;
		}
		if (_xs[i] < min[1] + _checkTime && _xs[i] > min[1])
		{
			total[1] += _ys[i];
			count[1]++;
		}
	}
	
	total[0] /= count[0];
	total[1] /= count[1];

//	std::unique_lock<std::mutex> lck(_mut);
//	emit changedFit();
//	_cv.wait(lck);

	return fabs(total[0] - total[1]);
}
