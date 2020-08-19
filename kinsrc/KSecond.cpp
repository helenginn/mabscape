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

#include "KSecond.h"
#include "Kintroller.h"
#include <RefinementNelderMead.h>
#include <FileReader.h>
#include <Any.h>

KSecond::KSecond()
{
	_second = NULL;
	_active = false;

	_jOn = 0;
	_jOff = 0;
	_jTot = 1;
	_jRatio = 1;
}

void KSecond::refineCascade()
{
	KFirst::refineCascade();
	refine();
}

void KSecond::populateYs()
{
	KFirst::populateYs();

	if (!_active)
	{
		return;
	}

	double min; double max;
	curve()->getRegion(1, &min, &max);
	
	double total = _jTot;
	double on = _jOn;
	double off = _jOff;

	if (_jRatio > 0)
	{
		total /= _jRatio;
		on *= _jRatio;
		off *= _jRatio;
	}

	double ks = on * total;
	for (size_t i = 0; i < _ys.size(); i++)
	{
		if (_xs[i] > min && _xs[i] < max)
		{
			double t0 = _xs[i] - min;
			double y = 0;
			double part1 = exp(-off * t0 - on * t0) / (off + on);
			double part2 = ks * exp(on * t0 + off * t0) - ks;
			y = part1 * part2 + _jCut;
			_ys[i] = y;
		}
	}
}

void KSecond::addToStrategy(RefinementStrategyPtr str)
{
	AnyPtr any;

	any = AnyPtr(new Any(&_jTot));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.2, 0.000001);

	if (_onOffOnly)
	{
		return;
	}

	any = AnyPtr(new Any(&_jOff));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.0002, 0.000001);

	any = AnyPtr(new Any(&_jOn));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.0005, 0.00001);

	/*
	any = AnyPtr(new Any(&_jCut));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.1, 0.0001);
	*/

	any = AnyPtr(new Any(&_jRatio));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 0.1, 0.0001);

	/*
	double *boundary = curve()->regionPtr(1, true);
	any = AnyPtr(new Any(boundary));
	_anys.push_back(any);
	str->addParameter(&*any, Any::get, Any::set, 2.0, 0.0001);
	*/
}

void KSecond::refine()
{
	double min, max;
	curve()->getRegion(1, &min, &max);
	_jCut = _topLigand;
	
	_jOn = _second->getKOn();
	_jOff = _second->getKOff();
	_jTot = _second->getKTotal();

	_onOffOnly = true;
	_active = true;

	NelderMeadPtr mead = NelderMeadPtr(new RefinementNelderMead());
	mead->setSilent();
	KSecond::addToStrategy(mead);
	mead->setEvaluationFunction(KModel::getScore, this);
	mead->setCycles(80);
	mead->refine();
	mead->setCycles(80);
	mead->refine();
	mead->setCycles(150);
	mead->refine();

	_onOffOnly = false;
	mead = NelderMeadPtr(new RefinementNelderMead());
	mead->setSilent();
	KSecond::addToStrategy(mead);
	mead->setEvaluationFunction(KModel::getScore, this);
	mead->setCycles(80);
	mead->refine();
	mead->setCycles(150);
	mead->refine();
	
	while (mead->changedSignificantly())
	{
		mead = NelderMeadPtr(new RefinementNelderMead());
		mead->setSilent(true);
		KLigOnOff::addToStrategy(mead);
		mead->setEvaluationFunction(KModel::getScore, this);
		mead->setCycles(500);
		mead->refine();
		std::cout << "refined..." << std::endl;
	}

	std::cout << "Final results: " << std::endl;
	_jTot /= _jRatio;
	_jOn *= _jRatio;
	_jOff *= _jRatio;
	_jRatio = 1;
	
	std::cout << "Max substrate response: " << _jTot 
	<< " response units." << std::endl;
	std::cout << "K_on: " << _jOn << " inverse time units." << std::endl;
	std::cout << "K_off: " << _jOff << " inverse time units." << std::endl;
	std::cout << std::endl;
	
	std::cout << "ks for now: " << _jOn * _jTot << std::endl;
	double kTot = _second->getKTotal();
	double kOn  = _second->getKOn();

	std::cout << "ks for then: " << kOn * kTot << std::endl;
	double ratio = _jTot / kTot;
	std::cout << "ratio: " << ratio << std::endl;
	std::cout << " comp: " << 1 - ratio << std::endl;

	std::string str = _first->nickname() + "," + _second->nickname()
	+ "," + f_to_str(1 - ratio, 3);

	std::cout << str << std::endl;

	Kintroller::theKintroller()->collectResult(str);
}
