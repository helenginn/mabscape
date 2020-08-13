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

#include "KCorrectExp.h"
#include <RefinementNelderMead.h>
#include <Any.h>
#include <iostream>

KCorrectExp::KCorrectExp()
{
	_decay = NULL;
	_correctX = true;
}

bool KCorrectExp::check()
{
	if (_decay == NULL)
	{
		return false;
	}

	return true;
}

void KCorrectExp::refineCascade()
{
	if (!check())
	{
		return;
	}

	KCorrectExp::refine();
	KOffset::refine();
	curveChinUp();
}

void KCorrectExp::curveChinUp()
{
	double exponent = _decay->getExponent();

	_ys = curve()->ys();
	KOffset::populateYs();
	
	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	double height = _decay->getStart() - _decay->getYOffset();

	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[0] && _xs[i] < max[1])
		{
			double actual = exp(-exponent * (_xs[i] - min[0]));
			actual *= height;
			actual += _decay->getYOffset();

			_ys[i] -= actual;
		}
	}
}

void KCorrectExp::addToStrategy(RefinementStrategyPtr str)
{
	double *boundary = curve()->regionPtr(0, true);
	AnyPtr any = AnyPtr(new Any(boundary));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.2, 0.0001);
}

void KCorrectExp::refine()
{
	if (!_correctX)
	{
		return;
	}

	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	_ys = curve()->ys();

	NelderMeadPtr mead = NelderMeadPtr(new RefinementNelderMead());
	KCorrectExp::addToStrategy(mead);
	mead->setEvaluationFunction(KCorrectExp::getScore, this);
	mead->setCycles(50);
	mead->refine();
}

double KCorrectExp::score()
{
	const double checkTime = 3;
	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	double bottomSum = 0;
	double bottomCount = 0;
	double topSum = 0;
	double topCount = 0;

	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[0] - checkTime && _xs[i] < min[0])
		{
			bottomSum += _ys[i];
			bottomCount++;
		}
		if (_xs[i] < min[0] + checkTime && _xs[i] > min[0])
		{
			topSum += _ys[i];
			topCount++;
		}
	}

	topSum /= topCount;
	bottomSum /= bottomCount;
	_yBottom = bottomSum;
	
	return -fabs(topSum - bottomSum);
}

