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
#include "SurfaceView.h"
#include "Experiment.h"
#include "Explorer.h"
#include <iostream>
#include <QThread>
#include <QApplication>

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

void Controller::run()
{
	for (size_t i = 0; i < _args.size(); i++)
	{
		std::string first, last;
		splitCommand(_args[i], &first, &last);
		
		if (first == "load-structure")
		{
			std::string fn = last;
			_exp->loadStructure(fn);
		}
		if (first == "load-structure-coords")
		{
			std::string fn = last;
			_exp->loadStructureCoords(fn);
		}
		else if (first == "refine-mesh")
		{
			QThread *w = _exp->meshStructure();
			w->wait();
		}
		else if (first == "triangulate-mesh")
		{
			_exp->triangulateMesh();
		}
		else if (first == "triangulate-structure")
		{
			_exp->triangulateStructure();
		}
		else if (first == "load-data")
		{
			std::string fn = last;
			_exp->loadCSV(fn);
		}
		else if (first == "load-positions")
		{
			std::string fn = last;
			_exp->loadPositions(fn);
		}
		else if (first == "monte-carlo")
		{
			int cycles = atoi(last.c_str());
			_exp->setMonteTarget(cycles);
			QThread *w = _exp->monteCarlo();
			w->wait();
		}
		else if (first == "write-results")
		{
			std::string fn = last;
			_exp->getExplorer()->writeResultsToFile(fn, true);
		}
		else if (first == "patchwork")
		{
			std::string ab = last;

			connect(this, &Controller::startPatch, _exp,
			        [=]() {_exp->abPatchwork(ab);},
			        Qt::QueuedConnection);
			
			emit startPatch();
		}
		else if (first == "quit")
		{
			QApplication::quit();
		}
	}
}
