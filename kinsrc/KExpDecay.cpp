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

#include "KExpDecay.h"
#include <Any.h>
#include <RefinementNelderMead.h>
#include <iostream>

KExpDecay::KExpDecay()
{
	_start = 0;
	_exponent = 0;
	_yOffset = 0;
}

void KExpDecay::refineCascade()
{
	KOffset::refine();
	curve()->copyYs(this);
	KExpDecay::refine();
	std::cout << "Done" << std::endl;
}

void KExpDecay::refine()
{
	if (!check())
	{
		return;
	}

	_exponent = 0;
	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[0])
		{
			_start = _ys[i];
			break;
		}
	}

	_yOffset = 0.;
	for (size_t i = _ys.size() - 1; i > 0; i--)
	{
		if (_xs[i] < max[1])
		{
			_yOffset = _ys[i];
			break;
		}
	}
	
	_exponent = log(2) / (max[1] - min[0]);

	NelderMeadPtr mead = NelderMeadPtr(new RefinementNelderMead());
	mead->setSilent(true);
	KExpDecay::addToStrategy(mead);
	mead->setEvaluationFunction(KModel::getScore, this);
	mead->setCycles(50);
	mead->refine();
	mead->setCycles(100);
	mead->refine();
	AnyPtr any = AnyPtr(new Any(&_start));
	_anys.push_back(any);
	mead->addParameter(&*any, Any::get, Any::set, 0.02, 0.00001);
	mead->setCycles(100);
	mead->refine();
	
	std::cout << "Half life: " << log(2) / _exponent << 
	" units of time." << std::endl;
	std::cout << "Y offset: " << _yOffset << " response units." << std::endl;
	std::cout << "Y start: " << _start << " response units." << std::endl;
}

bool KExpDecay::check()
{
	bool set = curve()->isRegionSet(0);
	if (!set)
	{
		std::cout << "Region 1 in curve not set." << std::endl;
		return false;
	}

	return true;
}

void KExpDecay::addToStrategy(RefinementStrategyPtr str)
{
	AnyPtr any = AnyPtr(new Any(&_exponent));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.0002, 0.0000001);

	any = AnyPtr(new Any(&_yOffset));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.1, 0.00001);
}

void KExpDecay::populateYs()
{
	_ys = curve()->ys();

	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);
	
	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[0] && _xs[i] < max[1])
		{
			_ys[i] = exp(-_exponent * (_xs[i] - min[0]));
			_ys[i] *= (_start - _yOffset);
			_ys[i] += _yOffset;
		}
	}
}
