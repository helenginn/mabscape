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

#include "Controller.h"
#include "Metadata.h"
#include "SurfaceView.h"
#include "Experiment.h"
#include "Structure.h"
#include "Explorer.h"
#include <h3dsrc/Mesh.h>
#include <iostream>
#include <QThread>
#include <QApplication>
#include <hcsrc/FileReader.h>

Controller::Controller()
{
	_view = NULL;

}

void Controller::setCommandLineArgs(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string str = argv[i];
		_args.push_back(str);
	}
}

void Controller::setView(SurfaceView *view)
{
	_view = view;
	_exp = view->getExperiment();
}

void splitCommand(std::string command, std::string *first, std::string *last)
{
	size_t equal = command.find('=');
	if (equal == std::string::npos)
	{
		*first = command;
		*last = "";
		return;
	}

	*first = command.substr(0, equal);
	
	if (command.length() <= equal + 1)
	{
		*last = "";
		return;
	}

	*last = command.substr(equal + 1, std::string::npos);
}

void Controller::jobDone()
{
	disconnect(_w, SIGNAL(finished()), this, SLOT(jobDone()));
	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	std::cout << "Done long job." << std::endl;
	incrementJob();
}

/* returns if should process next job immediately */
bool Controller::processNextArg(std::string arg)
{
	std::cout << std::endl;
	std::cout << "Processing next arg: " << "\"" << arg << 
	"\"" << std::endl;
	std::string first, last;
	splitCommand(arg, &first, &last);

	if (first == "load-structure")
	{
		std::string fn = last;
		_exp->loadStructure(fn);
		return true;
	}
	if (first == "load-structure-coords")
	{
		std::string fn = last;
		_exp->loadStructureCoords(fn);
		return true;
	}
	else if (first == "refine-mesh")
	{
		_exp->meshStructure();
		_exp->prepareWorkForMesh();
		Mesh *m = _exp->structure()->mesh();
		QThread *w = _exp->worker();
		connect(this, SIGNAL(refine()), m, SLOT(shrinkWrap()));
		connect(w, SIGNAL(finished()), this, SLOT(jobDone()));
		w->start();
		_w = w;
		std::cout << "Refining mesh" << std::endl;

		emit refine();
		
		return false;
	}
	else if (first == "smooth-mesh")
	{
		_exp->meshStructure();
		_exp->prepareWorkForMesh();
		Mesh *m = _exp->structure()->mesh();
		QThread *w = _exp->worker();
		connect(this, SIGNAL(refine()), m, SLOT(smoothCycles()));
		connect(w, SIGNAL(finished()), this, SLOT(jobDone()));
		w->start();
		_w = w;
		std::cout << "Smoothing mesh" << std::endl;

		emit refine();
		return false;
	}
	else if (first == "inflate-mesh")
	{
		_exp->meshStructure();
		_exp->prepareWorkForMesh();
		Mesh *m = _exp->structure()->mesh();
		QThread *w = _exp->worker();
		connect(this, SIGNAL(refine()), m, SLOT(inflateCycles()));
		connect(w, SIGNAL(finished()), this, SLOT(jobDone()));
		w->start();
		_w = w;
		std::cout << "Inflating mesh" << std::endl;

		emit refine();
		return false;
	}
	else if (first == "triangulate-mesh")
	{
		_exp->triangulateMesh();
		return true;
	}
	else if (first == "triangulate-structure")
	{
		_exp->triangulateStructure();
		return true;
	}
	else if (first == "load-data")
	{
		std::string fn = last;
		_exp->loadCSV(fn);
		emit fixMenu();
		return true;
	}
	else if (first == "load-positions")
	{
		std::string fn = last;
		_exp->loadPositions(fn);
		return true;
	}
	else if (first == "monte-carlo")
	{
		int cycles = atoi(last.c_str());
		_exp->setMonteTarget(cycles);
		QThread *w = _exp->worker();
		connect(w, SIGNAL(finished()), this, SLOT(jobDone()));
		_exp->monteCarlo();
		return false;
	}
	else if (first == "write-results")
	{
		std::string fn = last;
		_exp->getExplorer()->writeResultsToFile(fn, true);
		return true;
	}
	else if (first == "read-results" || first == "load-results")
	{
		std::string fn = last;
		_exp->getExplorer()->readResults(fn);
		return true;
	}
	else if (first == "select-all")
	{
		_exp->getExplorer()->selectAll();
		return true;
	}
	else if (first == "use-target")
	{
		if (last == "both")
		{
			_exp->chooseTarget(TargetBoth);
		}
		else if (last == "correlation")
		{
			_exp->chooseTarget(TargetCorrelation);
		}
		else if (last == "least-squares")
		{
			_exp->chooseTarget(TargetLeastSquares);
		}

		return true;
	}
	else if (first == "average-positions")
	{
		_exp->getExplorer()->summariseBounds();
		return true;
	}
	else if (first == "load-metadata")
	{
		Metadata *m = _exp->metadata();
		m->setFilename(last);
		m->loadBounds(_exp->bounds());
		m->load();

		return true;
	}
	else if (first == "make-antibody")
	{
		std::string file = last;
		std::vector<std::string> names = split(last, ',');
		for (size_t i = 0; i < names.size(); i++)
		{
			_exp->makeNamedBound(names[i]);
		}

		return true;
	}
	else if (first == "ab-junctions")
	{
		std::string file = last;
		_view->alignSequences();

		return true;
	}
	else if (first == "add-pdb")
	{
		_exp->structure()->addPDB(last);

		return true;
	}
	else if (first == "residue-cones")
	{
		_exp->structure()->conesForResidueList(_view->getGL(), last);

		return true;
	}
	else if (first == "heat-map")
	{
		_exp->heatMap();

		return true;
	}
	else if (first == "quit")
	{
		QApplication::quit();
		return true;
	}
	
	std::cout << "Didn't find a rule to process: " << first << std::endl;
	return true;
}

void Controller::incrementJob()
{
	if (_args.size() == 0)
	{
		return;
	}

	bool result = true;
	
	while (result)
	{
		result = processNextArg(_args[_currentJob]);
		_currentJob++;
		
		if ((size_t)_currentJob >= _args.size())
		{
			break;
		}
	}
}

void Controller::run()
{
	connect(this, &Controller::fixMenu, _view, &SurfaceView::updateMenu,
	        Qt::QueuedConnection);

	_currentJob = 0;
	incrementJob();
}
