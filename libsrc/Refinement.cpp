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

#include "Refinement.h"
#include "Experiment.h"
#include "Bound.h"
#include "Data.h"
#include <iostream>
#include <fstream>
#include <FileReader.h>
#include <maths.h>
#include <RefinementNelderMead.h>
#include <RefinementLBFGS.h>
#include "Structure.h"
#include "Mesh.h"

Target Refinement::_target = TargetLeastSquares;
Target Refinement::_currTarg = TargetLeastSquares;
bool Refinement::_relocateFliers = true;

Refinement::Refinement(Experiment *expt)
{
	_fixedOnly = false;
	_experiment = expt;
	_data = NULL;
	if (expt->getData())
	{
		_data = expt->getData();
	}
	_pause = false;
	_randomiseFirst = true;
	
	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *b = _experiment->bound(i);
		b->setSnapping(false);
	}
}

void Refinement::recolourByScore()
{
	std::map<Bound *, double> scores;
	CorrelData cdcc = empty_CD();
	CorrelData allcc = empty_CD();
	
	std::ofstream comp;
	comp.open("cc_comparison.csv");

	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		CorrelData cd = empty_CD();

		for (size_t j = 0; j < _experiment->boundCount(); j++)
		{
			Bound *bj = _experiment->bound(j);
			
			if (bi == bj)
			{
				continue;
			}

			double val = _data->valueFor(bi->name(), bj->name());
			
			if (val != val)
			{
				continue;
			}
			
			val = std::max(0., val); val = std::min(1., val);

			double raw = bi->scoreWithOther(bj);
			double diff = (raw - val) * (raw - val);

			if (diff != diff)
			{
				continue;
			}

			add_to_CD(&cd, raw, val);
			add_to_CD(&allcc, raw, val);
			
			comp << val << ", " << raw << std::endl;
		}

		double correl = evaluate_CD(cd);
		
		scores[bi] = correl;
		add_to_CD(&cdcc, correl, 0.);
	}
	
	comp.close();
	
	double m1, m2, s1, s2;
	means_stdevs_CD(cdcc, &m1, &m2, &s1, &s2);
	std::cout << "Mean CC per Ab: " << m1 << std::endl;
	std::cout << "Stdev CC per Ab: " << s1 << std::endl;
	
	double overall = evaluate_CD(allcc);
	std::cout << "Overall CC: " << overall << std::endl;


	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		double correl = scores[bi];
		correl -= m1;
		correl /= s1 * 2;
		correl += 0.5;
		correl = std::max(correl, 0.);
		correl = std::min(correl, 1.);
		bi->setColour(correl, correl, 0);
	}
}

double Refinement::partialScore(Bound *bi)
{
	double sum = 0;
	double count = 0;
	CorrelData cd = empty_CD();

	for (size_t j = 0; j < _experiment->boundCount(); j++)
	{
		Bound *bj = _experiment->bound(j);
		
		if (bi == bj)
		{
			continue;
		}

		double val = _data->valueFor(bi->name(), bj->name());
		
		if (val != val)
		{
			continue;
		}

		if (_fixedOnly && !bi->isFixed() && !bj->isFixed())
		{
			continue;
		}
		
		if (bi->isFixed() && bj->isFixed())
		{
			continue;
		}

		double raw = bi->scoreWithOther(bj);
		double diff = (raw - val) * (raw - val);

		if (diff != diff)
		{
			continue;
		}

		if (_currTarg == TargetLeastSquares)
		{
			sum += diff;
			count++;
		}

		add_to_CD(&cd, raw, val);
	}

	double correl = evaluate_CD(cd);
	if (_currTarg == TargetCorrelation)
	{
		return -correl;
	}
	else
	{
		double result = sum / count;
		
		if (result != result)
		{
			return 0;
		}

		return result;
	}
}

double Refinement::score()
{
	double sum = 0;
	double count = 0;
	CorrelData cd = empty_CD();
	double cc = 0;
	double abcount = 0;

	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		bi->getWorkingPosition();
	}

	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		
		if (!_pause)
		{
			bi->updatePositionToReal();
		}
		
		CorrelData cd_per_ab = empty_CD();
		std::string name = bi->name();

		for (size_t j = 0; j < _experiment->boundCount(); j++)
		{
			if (_currTarg == TargetLeastSquares && j >= i)
			{
				continue;
			}

			if (i == j)
			{
				continue;
			}
			
			Bound *bj = _experiment->bound(j);
			
			if (_fixedOnly && !bi->isFixed() && !bj->isFixed())
			{
				continue;
			}
			
			if (_fixedOnly && bi->isFixed() && bj->isFixed())
			{
				continue;
			}
			
			double val = _data->valueFor(bi->name(), bj->name());

			if (val != val)
			{
				continue;
			}

			bool shouldDampen = false;//_cycleNum >= 25;
			double raw = bi->scoreWithOther(bj, shouldDampen);

			double diff = (raw - val) * (raw - val);
			
			if (diff != diff)
			{
				continue;
			}
			
			if (_currTarg == TargetLeastSquares)
			{
				sum += diff;
				count++;
			}
			
			add_to_CD(&cd, raw, val);
			add_to_CD(&cd_per_ab, raw, val);
		}
		
		double correl = evaluate_CD(cd_per_ab);
		cc += correl;
		abcount++;
	}

	if (_currTarg == TargetCorrelation)
	{
		cc /= abcount;
		return -cc;
	}
	else
	{
		double result = sum / count;
		
		if (result != result)
		{
			return 0;
		}

		return result;
	}
}

void Refinement::refine()
{
	RefinementLBFGSPtr ref = RefinementLBFGSPtr(new RefinementLBFGS());

	bool changed = true;
	_cycleNum = 0;
	int maxCycles = 60;
	_fixedOnly = false;
	
	_currTarg = TargetCorrelation;
	if (_target == TargetLeastSquares || _target == TargetBoth)
	{
		_currTarg = TargetLeastSquares;
	}
	
	if (_randomiseFirst)
	{
		_fixedOnly = true;

		for (size_t i = 0; i < _experiment->boundCount(); i++)
		{
			Bound *b = _experiment->bound(i);
			b->randomlyPositionInRegion(_experiment->structure()->mesh());
		}
	}

	while (changed && _cycleNum < maxCycles)
	{
		ref->clearParameters();

		for (size_t i = 0; i < _experiment->boundCount(); i++)
		{
			Bound *b = _experiment->bound(i);
			b->addToStrategy(&*ref);
		}

		ref->setCycles(2000);
		ref->setEvaluationFunction(Refinement::getScore, this);
		ref->setPartialEvaluation(Refinement::getPartialScore);

		ref->setSilent(true);
		ref->refine();

		for (size_t i = 0; i < _experiment->boundCount() && !_fixedOnly; i++)
		{
			Bound *b = _experiment->bound(i);
			double dist = b->snapToObject(NULL);
			
			if (dist > Bound::getRadius() * 2 / 3 && 
			    _currTarg == TargetLeastSquares && _relocateFliers)
			{
				b->randomlyPositionInRegion(_experiment->structure()->mesh());
			}
		}

		if (_cycleNum <= 30)
		{
			_experiment->jiggle();
		}
		
		_cycleNum++;
		changed = ref->changedSignificantly();
		
		if (_cycleNum == 10)
		{
			_fixedOnly = false;
		}
		
		if (_cycleNum == 30 && _target == TargetCorrelation)
		{
			_currTarg = TargetCorrelation;
		}
		
		if (_cycleNum <= 30 && _target == TargetBoth)
		{
			changed = true;
		}
	}

	_currTarg = TargetLeastSquares;
	std::cout << score() << " / " << std::flush;
	_currTarg = TargetCorrelation;
	std::cout << -score() << std::flush;
	
	if (_target == TargetLeastSquares)
	{
		_currTarg = TargetLeastSquares;
	}

	std::cout << std::endl;

	resultReady();
}

double Refinement::compareBinders(void *obj, Parameter &p1, Parameter &p2)
{
	/* no correlation between x and y, only between x and x, etc */
	if (p1.setter != p2.setter)
	{
		return 0;
	}

	Bound *bi = static_cast<Bound *>(p1.object);
	Bound *bj = static_cast<Bound *>(p2.object);
	Refinement *ref = static_cast<Refinement *>(obj);
	double val = ref->_data->valueFor(bi->name(), bj->name());
	
	if (val != val) return 0;
	
	return val;
}
