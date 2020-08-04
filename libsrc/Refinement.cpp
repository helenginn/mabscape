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
#include <maths.h>
#include <RefinementNelderMead.h>
#include <RefinementLBFGS.h>

Target Refinement::_target = TargetLeastSquares;

Refinement::Refinement(Experiment *expt)
{
	_fixedOnly = false;
	_experiment = expt;
	_data = expt->getData();
	_convert = false;
	
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
			double raw = bi->scoreWithOther(bj);
			double diff = (raw - val) * (raw - val);

			if (diff != diff)
			{
				continue;
			}

			add_to_CD(&cd, raw, val);
		}

		double correl = evaluate_CD(cd);
		
		scores[bi] = correl;
		add_to_CD(&cdcc, correl, 0.);
	}
	
	double m1, m2, s1, s2;
	means_stdevs_CD(cdcc, &m1, &m2, &s1, &s2);
	std::cout << "Mean CC: " << m1 << std::endl;
	std::cout << "Stdev CC: " << s1 << std::endl;

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

		double val = _data->valueFor(bi->name(), bj->name());
		double raw = bi->scoreWithOther(bj);
		double diff = (raw - val) * (raw - val);

		if (diff != diff)
		{
			continue;
		}

		if (_fixedOnly && !bi->isFixed() && !bj->isFixed())
		{
			continue;
		}

		if (_target == TargetLeastSquares)
		{
			sum += diff;
			count++;
		}

		add_to_CD(&cd, raw, val);
	}

	double correl = evaluate_CD(cd);
	if (_target == TargetCorrelation)
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

	for (size_t i = 1; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		bi->updatePositionToReal();

		for (size_t j = 0; j < i; j++)
		{
			Bound *bj = _experiment->bound(j);
			
			double val = _data->valueFor(bi->name(), bj->name());
			double raw = bi->scoreWithOther(bj);
			double diff = (raw - val) * (raw - val);
			
			if (diff != diff)
			{
				continue;
			}
			
			if (_fixedOnly && !bi->isFixed() && !bj->isFixed())
			{
				continue;
			}
			
			if (_target == TargetLeastSquares)
			{
				sum += diff;
				count++;
			}
			
			add_to_CD(&cd, raw, val);
		}
	}

	double correl = evaluate_CD(cd);
	if (_target == TargetCorrelation)
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

void Refinement::refine()
{
	RefinementLBFGSPtr ref = RefinementLBFGSPtr(new RefinementLBFGS());

	bool changed = true;
	int count = 0;
	int target = 10;
	if (!_fixedOnly && !_convert)
	{
		target = 100;
	}
	while (changed && count < target)
	{
		ref->clearParameters();

		for (size_t i = 0; i < _experiment->boundCount(); i++)
		{
			Bound *b = _experiment->bound(i);
			b->addToStrategy(&*ref);
		}

		ref->setCycles(2000);
		ref->setEvaluationFunction(Refinement::getScore, this);
		
		if (!_convert)
		{
			ref->setPartialEvaluation(Refinement::getPartialScore);
		}

		ref->setSilent(true);
		
		Converter *conv = NULL;
		if (_convert)
		{
			conv = new Converter();
			conv->setCompareFunction(this, compareBinders);
			conv->setStrategy(ref);
		}

		ref->refine();

		delete conv;

		for (size_t i = 0; i < _experiment->boundCount(); i++)
		{
			Bound *b = _experiment->bound(i);
			b->snapToObject(NULL);
		}
		
		count++;
		changed = ref->changedSignificantly();
		std::cout << " ... " << score() << std::flush;
		
		if (_fixedOnly && count == 4)
		{
			_fixedOnly = false;
		}
	}

	std::cout << std::endl;

	Bound::updateOnRender(false);
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
