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
#include <RefinementNelderMead.h>
#include <RefinementLBFGS.h>

Refinement::Refinement(Experiment *expt, bool restrained)
{
	_experiment = expt;
	_data = expt->getData();
	
	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *b = _experiment->bound(i);
		b->setSnapping(restrained);
	}
	
	std::cout << "Progress: " << score() << "..." << std::flush;
}

double Refinement::score()
{
	double radius = Bound::getRadius();
	double sum = 0;
	double count = 0;

	for (size_t i = 1; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		std::string bin = bi->name();
		vec3 posi = bi->getWorkingPosition();
		for (size_t j = 0; j < i; j++)
		{
			Bound *bj = _experiment->bound(j);
			std::string bjn = bj->name();

			double val = _data->valueFor(bin, bjn);
			if (val != val)
			{
				continue;
			}
			
			vec3 posj = bj->getWorkingPosition();

			vec3 vector = vec3_subtract_vec3(posi, posj);
			double distance = vec3_length(vector);
			
			/*
			double prop = 0;
			if (distance < 2 * radius)
			{
				double q = (2 * radius - distance) / 2;
				prop = 2 * (3 * q * q - 2 * q * q * q);
			}
			*/
			
			double prop = exp(-(distance * distance) / (2 * radius));
			count++;
			
			double diff = val - prop;
			diff *= diff;
			sum += diff;
		}
	}

	return sum / count;
}

void Refinement::refine()
{
	std::cout << "Refining..." << std::endl;
	Bound::updateOnRender(true);
	
	RefinementLBFGS *ref = new RefinementLBFGS();

	bool changed = true;
	int count = 0;
	while (changed && count < 10)
	{
		ref->clearParameters();

		for (size_t i = 0; i < _experiment->boundCount(); i++)
		{
			Bound *b = _experiment->bound(i);
			b->addToStrategy(ref);
		}

		ref->setCycles(2000);
		ref->setEvaluationFunction(Refinement::getScore, this);
		ref->setSilent(true);
		ref->refine();

		for (size_t i = 0; i < _experiment->boundCount(); i++)
		{
			Bound *b = _experiment->bound(i);
			b->snapToObject(NULL);
		}
		
		count++;
		changed = ref->changedSignificantly();
		std::cout << " ... " << score() << std::flush;
	}
	std::cout << std::endl;

	Bound::updateOnRender(false);
	resultReady();
}

