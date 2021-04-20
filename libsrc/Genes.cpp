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

#include "Genes.h"
#include "Metadata.h"
#include "Antibody.h"
#include "Experiment.h"
#include "SurfaceView.h"
#include <fstream>
#include <hcsrc/FileReader.h>
#include <c4xsrc/AveCSV.h>
#include <c4xsrc/Screen.h>
#include <c4xsrc/ClusterList.h>
#include <c4xsrc/Group.h>

Genes::Genes()
{

}

double Genes::valueFor(bool heavy, Bound *b, Bound *c)
{
	std::map<Bound *, BoundValue> *map = &_heavy;
	if (!heavy)
	{
		map = &_light;
	}

	if (!map->count(b) || !(*map)[b].count(c))
	{
		return NAN;
	}

	return (*map)[b][c];
}

void Genes::compare(AveCSV *csv, bool heavy)
{
	for (size_t i = 1; i < _abs.size(); i++)
	{
		for (size_t j = 0; j < i; j++)
		{
			double val = _abs[i]->compareWithAntibody(_abs[j], heavy);
			csv->addValue(_abs[j]->name(), _abs[i]->name(), val);
			csv->addValue(_abs[i]->name(), _abs[j]->name(), val);
			
			if (heavy)
			{
				_heavy[_abs[i]->bound()][_abs[j]->bound()] = val;
				_heavy[_abs[j]->bound()][_abs[i]->bound()] = val;
			}
			else
			{
				_light[_abs[i]->bound()][_abs[j]->bound()] = val;
				_light[_abs[j]->bound()][_abs[i]->bound()] = val;
			}
		}
	}
	
	std::cout << "Updated " << (heavy ? "heavy " : "light ") <<
	"sequence comparison" << std::endl;
}

void Genes::loadSequences(Metadata *m, SurfaceView *view)
{
	for (size_t i = 0; i < m->boundCount(); i++)
	{
		Bound *b = m->bound(i);
		
		std::string haa = m->valueForKey(b, "heavy");
		std::string laa = m->valueForKey(b, "light");
		
		if (haa.length() == 0 && laa.length() == 0)
		{
			continue;
		}
		
		Antibody *ab = new Antibody(b, haa, laa);
		_abs.push_back(ab);
	}
	
	std::cout << _abs.size() << " antibodies loaded" << std::endl;
	std::cout << std::endl;
	
	bool heavy = m->hasTitle("heavy");
	bool light = m->hasTitle("light");
	
	view->launchCluster4x();
	Screen *scr = view->clusterScreen();
	ClusterList *list = scr->getList();
	AveCSV *csv = Group::topGroup()->getAveCSV();
	csv->setList(list);
	
	if (heavy)
	{
		csv->startNewCSV("Heavy chain");
		compare(csv, true);
	}
	
	if (light)
	{
		csv->startNewCSV("Light chain");
		compare(csv, false);
	}
	
	if (heavy || light)
	{
		csv->setChosen(csv->csvCount() - 1);
	}
	
	if (Group::topGroup()->mtzCount() == 0)
	{
		csv->preparePaths();
		list->addCSVSwitcher();
	}
}

