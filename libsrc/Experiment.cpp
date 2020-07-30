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

#include "Experiment.h"
#include "SurfaceView.h"
#include "SlipGL.h"
#include "Bound.h"
#include "Data.h"
#include "Structure.h"
#include "Mesh.h"
#include "Refinement.h"
#include "FileReader.h"
#include <iomanip>
#include <fstream>
#include <float.h>
#include <QLabel>
#include <QMenu>
#include <QThread>

Experiment::Experiment(SurfaceView *view)
{
	_mesh = NULL;
	_bestMonte = FLT_MAX;
	_worker = NULL;
	_data = NULL;
	_selected = NULL;
	_monteCount = -1;
	_dragging = false;
	_view = view;
	_gl = NULL;
	_structure = NULL;
	_label = new QLabel("", _view);
	_label->setAlignment(Qt::AlignVCenter);
	_label->setAlignment(Qt::AlignHCenter);
	QFont font = QFont("Helvetica", 16);
	_label->setFont(font);
	_label->setStyleSheet("QLabel { color : white; }");
}


void Experiment::loadStructure(std::string filename)
{
	Structure *str = new Structure(filename);
	_gl->addObject(str, true);
	_structure = str;
}

void Experiment::meshStructure()
{
	Mesh *mesh = _structure->makeMesh();
	_gl->addObject(mesh, false);
	_mesh = mesh;

	refineMesh();
}

void Experiment::triangulateMesh()
{
	if (_mesh == NULL)
	{
		return;
	}
	
	_mesh->changeToTriangles();
	_mesh->SlipObject::triangulate();
	_mesh->changeToLines();
}

void Experiment::refineMesh()
{
	if (_mesh == NULL)
	{
		return;
	}

	if (_worker && _worker->isRunning())
	{
		return;
	}
	
	if (!_worker)
	{
		_worker = new QThread();
	}

	_mesh->moveToThread(_worker);

	connect(this, SIGNAL(refine()), _mesh, SLOT(shrinkWrap()));
	connect(_mesh, SIGNAL(resultReady()), this, SLOT(handleMesh()));

	_worker->start();

	emit refine();
}


Bound *Experiment::loadBound(std::string filename)
{
	Bound *bnd = new Bound(filename);
	bnd->setName("thing" + i_to_str(_bounds.size() + 1));
	_gl->addObject(bnd, false);
	
	if (_structure)
	{
		bnd->randomlyPositionInRegion(_structure);
		bnd->snapToObject(_structure);
		bnd->setStructure(_structure);
	}

	_bounds.push_back(bnd);
	return bnd;
}

Bound *Experiment::findBound(double x, double y)
{
	Bound *which = NULL;
	double z = -FLT_MAX;

	for (size_t i = 0; i < _bounds.size(); i++)
	{
		bool covers = _bounds[i]->intersects(x, y, &z);
		
		if (covers)
		{
			which = _bounds[i];
		}
	}

	return which;
}

void Experiment::hoverMouse(double x, double y)
{
	Bound *which = findBound(x, y);

	if (which != NULL)
	{
		dehighlightAll();
		which->setHighlighted(true);
	}
	else
	{
		dehighlightAll();
	}
}

void Experiment::dehighlightAll()
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		_bounds[i]->setHighlighted(false);
	}
}

void Experiment::deselectAll()
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		_bounds[i]->setSelected(false);
	}
	
	_selected = NULL;
	hideLabel();
}

void Experiment::select(Bound *bound, double x, double y)
{
	_selected = bound;
	bound->setSelected(true);
	QString name = QString::fromStdString(bound->name());
	_view->convertToViewCoords(&x, &y);

	if (x > 0 && y > 0)
	{
		_label->setGeometry(x - 50, y - 50, 100, 50);
		_label->setText(name);
		_label->show();
	}
}

void Experiment::checkDrag(double x, double y)
{
	if (_selected == NULL || _selected->isFixed())
	{
		_dragging = false;
		return;
	}

	double z = -FLT_MAX;

	_dragging = _selected->intersects(x, y, &z);
}

void Experiment::drag(double x, double y)
{
	if (_selected == NULL || !_dragging)
	{
		return;
	}

	_selected->changeMidPoint(x, y);
}

void Experiment::clickMouse(double x, double y)
{
	Bound *which = findBound(x, y);

	if (which != NULL)
	{
		std::cout << "Selected: " << which->name() << std::endl;
		deselectAll();
		select(which, x, y);
	}
	else
	{
		deselectAll();
	}
}

void Experiment::hideLabel()
{
	_label->setText("");
	_label->hide();
}

void Experiment::finishDragging()
{
	_dragging = false;
	if (_selected)
	{
		_selected->snapToObject(_structure);
	}
}

void Experiment::fixBound()
{
	if (_selected)
	{
		_selected->toggleFixPosition();
	}
}

void Experiment::loadCSV(std::string filename)
{
	_data = new Data(filename);
	createBinders();
}

void Experiment::createBinders()
{
	for (size_t i = 0; i < _data->boundCount(); i++)
	{
		std::string id = _data->boundName(i);
		std::cout << "Making " << id << std::endl;
		Bound *bound = loadBound(_boundObj);
		bound->setName(id);
		_nameMap[id] = bound;
	}
}

void Experiment::addBindersToMenu(QMenu *binders)
{
	for (size_t i = 0; i < _bounds.size(); i += 10)
	{
		size_t end = std::min(i + 10, _bounds.size());

		std::string title = "Binders " + i_to_str(i + 1) + " to " 
		+ i_to_str(end);

		QMenu *m = binders->addMenu(QString::fromStdString(title));
		_view->addMenu(m);

		for (size_t j = 0; i + j < _bounds.size() && j < 10; j++)
		{
			std::string name = _bounds[i + j]->name();
			QString n = QString::fromStdString(name);
			QAction *act = m->addAction(n);
			connect(act, &QAction::triggered, 
			        this, &Experiment::selectFromMenu);
			_view->addAction(act);
		}
	}
}

void Experiment::selectFromMenu()
{
	QObject *obj = QObject::sender();
	QAction *act = static_cast<QAction *>(obj);

	std::string name = act->text().toStdString();
	Bound *b = _nameMap[name];

	if (b == NULL)
	{
		return;
	}

	deselectAll();
	select(b, -1, -1);
}

void Experiment::refineModel(bool toSurface)
{
	if (_worker && _worker->isRunning())
	{
		return;
	}
	
	if (!_worker)
	{
		_worker = new QThread();
	}

	_refinement = new Refinement(this, toSurface);
	_refinement->moveToThread(_worker);

	connect(this, SIGNAL(refine()),
	        _refinement, SLOT(refine()));

	connect(_refinement, SIGNAL(resultReady()), 
	        this, SLOT(handleResults()));
	connect(_refinement, SIGNAL(failed()), 
	        this, SLOT(handleError()));
	_worker->start();

	emit refine();
}

void Experiment::handleResults()
{
	std::cout << "Done." << std::endl;
	Refinement *obj = static_cast<Refinement *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), this, SLOT(handleResults()));
	disconnect(obj, SIGNAL(failed()), this, SLOT(handleError()));

	obj->deleteLater();
	_worker = NULL;
	
	if (_monteCount >= 0 && _monteCount < 20)
	{
		double newest = Refinement::getScore(_refinement);
		std::cout << "New score: " << newest << std::endl;
		
		if (newest < _bestMonte)
		{
			std::cout << "Best so far!" << std::endl;
			savePositions(&_bestPositions);
			_bestMonte = newest;
		}
		_monteCount++;

		monteCarloRound();
	}
	else if (_monteCount >= 20)
	{
		applyPositions(_bestPositions);
		std::cout << "Choosing best score: " << _bestMonte << std::endl;
		_monteCount = -1;
	}
}

void Experiment::handleMesh()
{
	Mesh *obj = static_cast<Mesh *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), this, SLOT(handleMesh()));

	_worker = NULL;
}

void Experiment::handleError()
{
	Refinement *obj = static_cast<Refinement *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), this, SLOT(handleResults()));
	disconnect(obj, SIGNAL(failed()), this, SLOT(handleError()));

	obj->deleteLater();
	_worker = NULL;
}

void Experiment::randomise()
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		_bounds[i]->randomlyPositionInRegion(_structure);
		_bounds[i]->snapToObject(_structure);
	}
}

void Experiment::jiggle()
{
	for (int i = 0; i < 8; i++)
	{
		for (size_t i = 0; i < _bounds.size(); i++)
		{
			_bounds[i]->jiggleOnSurface(_structure);
		}
	}

}

void Experiment::monteCarloRound()
{
	std::cout << "Monte Carlo round " << _monteCount + 1 << std::endl;
	randomise();
	refineModel(false);
}

void Experiment::savePositions(std::map<Bound *, vec3> *posMap)
{
	posMap->clear();

	for (size_t i = 0; i < _bounds.size(); i++)
	{
		vec3 wip = _bounds[i]->getWorkingPosition();
		(*posMap)[_bounds[i]] = wip;
	}
}

void Experiment::applyPositions(std::map<Bound *, vec3> posMap)
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		if (posMap.count(_bounds[i]) == 0)
		{
			continue;
		}

		vec3 wip = posMap[_bounds[i]];
		_bounds[i]->setRealPosition(wip);
		_bounds[i]->updatePositionToReal();
	}
}

void Experiment::loadPositions(std::string filename)
{
	std::string contents = get_file_contents(filename);
	
	std::vector<std::string> lines = split(contents, '\n');
	
	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> bits = split(lines[i], ',');
		
		if (bits.size() < 4)
		{
			continue;
		}
		
		std::string name = bits[0];
		bool fixed = false;
		bool colour = false;
		
		if (name[0] == '*' && name.length() > 1)
		{
			name = &bits[0][1];
			fixed = true;
		}
		
		if (name[0] == '^' && name.length() > 1)
		{
			name = &bits[0][1];
			colour = true;
		}

		float v1 = atof(bits[1].c_str());
		float v2 = -atof(bits[2].c_str());
		float v3 = atof(bits[3].c_str());

		vec3 pos = make_vec3(v1, v2, v3);
		Bound *b = _nameMap[name];
		
		if (b == NULL)
		{
			continue;
		}

		b->setRealPosition(pos);

		if ((fixed && !b->isFixed()) ||
		    (!fixed && b->isFixed()))
		{
			b->toggleFixPosition();
		}

		if (colour)
		{
			b->recolour(0.8, 0.0, 0.0);
		}

		b->updatePositionToReal();
	}

}

void Experiment::monteCarlo()
{
	_monteCount = 0;

	monteCarloRound();

}

void Experiment::writeOutCSV()
{
	std::ofstream posicsv;
	posicsv.open("positions.csv"); 
	std::ofstream file;
	file.open("model.csv"); 

	for (size_t i = 0; i < boundCount(); i++)
	{
		Bound *bi = bound(i);
		std::string bin = bi->name();
		vec3 posi = bi->getWorkingPosition();
		std::string fixed = bi->isFixed() ? "*" : "";
		
		posicsv << fixed << bin << "," << posi.x << "," << -posi.y << ","
		<< posi.z << std::endl;

		for (size_t j = 0; j < i; j++)
		{
			Bound *bj = bound(j);
			std::string bjn = bj->name();
			double prop = 0;
			bi->scoreWithOther(bj, _data, &prop, true);

			file << bin << "," << bjn << "," << prop << std::endl;
		}
	}
	
	file.close();
	posicsv.close();
}

