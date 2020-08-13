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

#include "KLigOnOff.h"
#include <RefinementNelderMead.h>
#include <Any.h>

KLigOnOff::KLigOnOff()
{
	_lockOff = false;
	_totalSubstrate = 0;
	_ratio = 1;
	_kOff = 0;
	_kOn = 0;
	_yCut = 0;
	_onOffOnly = true;
	_mult = 1;
}

bool KLigOnOff::check()
{
	return true;
}


void KLigOnOff::refineCascade()
{
	KCorrectExp::refine();
	KOffset::refine();
	curveChinUp();
	curve()->copyYs(this);
	KLigOnOff::refine();
}

void KLigOnOff::addToStrategy(RefinementStrategyPtr str)
{
	AnyPtr any;

	if (_lockOff)
	{
		any = AnyPtr(new Any(&_yCut));
		_anys.push_back(any);
		str->addParameter(&*any, Any::get, Any::set,
		                  0.1 * _mult, 0.0001);

		double *boundary = curve()->regionPtr(0, true);
		any = AnyPtr(new Any(boundary));
		_anys.push_back(any);
		str->addParameter(&*any, Any::get, Any::set, 
		                  2.0 * _mult, 0.0001);

		any = AnyPtr(new Any(&_totalSubstrate));
		_anys.push_back(any);
		str->addParameter(&*any, Any::get, Any::set, 
		                  0.1 * _mult, 0.0001);
		
		return;
	}

	any = AnyPtr(new Any(&_kOff));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set,
	                  0.0002 * _mult, 0.000001);

	any = AnyPtr(new Any(&_kOn));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set,
	                  0.0005 * _mult, 0.00001);

	any = AnyPtr(new Any(&_ratio));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set,
	                  0.1 * _mult, 0.0001);

	if (_onOffOnly)
	{
		return;
	}

	any = AnyPtr(new Any(&_yCut));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set,
	                  0.1 * _mult, 0.0001);

	double *boundary = curve()->regionPtr(0, true);
	any = AnyPtr(new Any(boundary));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 
	                  2.0 * _mult, 0.0001);
}

void KLigOnOff::refine()
{
	_yCut = 0;
	_mult = 1;

	_totalSubstrate = 0;
	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);

	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min[0] && _xs[i] < max[1])
		{
			if (_ys[i] > _totalSubstrate)
			{
				_totalSubstrate = _ys[i];
			}
		}
	}
	
	_totalSubstrate *= 2;
	if (!_lockOff)
	{
		_kOn = 0.005;
		_kOff = 0.0005;
	}
	
	std::cout << "Starting substrate: " << _totalSubstrate << std::endl;

	_onOffOnly = true;

	NelderMeadPtr mead = NelderMeadPtr(new RefinementNelderMead());
	KLigOnOff::addToStrategy(mead);
	mead->setEvaluationFunction(KModel::getScore, this);
	mead->setCycles(80);
	mead->refine();
	mead->setCycles(80);
	mead->refine();
	_mult *= 0.9;

	_onOffOnly = false;
	mead = NelderMeadPtr(new RefinementNelderMead());
	KLigOnOff::addToStrategy(mead);
	mead->setEvaluationFunction(KModel::getScore, this);
	mead->setCycles(80);
	mead->refine();
	mead->setCycles(150);
	mead->refine();
	_mult *= 0.9;
	
	while (mead->changedSignificantly())
	{
		mead = NelderMeadPtr(new RefinementNelderMead());
		KLigOnOff::addToStrategy(mead);
		mead->setEvaluationFunction(KModel::getScore, this);
		mead->setCycles(500);
		mead->refine();
		_mult *= 0.9;
	}
	
	std::cout << "Final results: " << std::endl;
	_totalSubstrate /= _ratio;
	_kOn *= _ratio;
	_kOff *= _ratio;
	
	std::cout << "Max substrate response: " << _totalSubstrate 
	<< " response units." << std::endl;
	std::cout << "K_on: " << _kOn << " inverse time units." << std::endl;
	std::cout << "K_off: " << _kOff << " inverse time units." << std::endl;
	std::cout << std::endl;
}

void KLigOnOff::populateYs()
{
	_ys = curve()->ys();

	double min[2]; double max[2];
	curve()->getRegion(0, &min[0], &max[0]);
	curve()->getRegion(1, &min[1], &max[1]);
	
	double total = _totalSubstrate;
	double on = _kOn;
	double off = _kOff;

	if (_ratio > 0)
	{
		total /= _ratio;
		on *= _ratio;
		off *= _ratio;
	}

	_topLigand = 0;
	double ks = on * total;
	for (size_t i = 0; i < _ys.size(); i++)
	{

		if (_xs[i] > min[0] && _xs[i] < max[0])
		{
			double t0 = _xs[i] - min[0];
			double y = 0;
			double part1 = exp(-off * t0 - on * t0) / (off + on);
			double part2 = ks * exp(on * t0 + off * t0) - ks;
			y = part1 * part2 + _yCut;
			_ys[i] = y;
			
			_topLigand = y;
		}
		else if (_xs[i] > min[1] && _xs[i] < max[1])
		{
			double t1 = _xs[i] - min[1];
			double y = ks * exp(-off * t1) / (off + on) + _yCut;
			double y0 = ks / (off + on) + _yCut;
			double diff = y0 - _topLigand;
			_ys[i] = y - diff;
		}
	}
}

