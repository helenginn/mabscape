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
#include "Explorer.h"
#include "SurfaceView.h"
#include "SlipGL.h"
#include "Bound.h"
#include "Data.h"
#include "Structure.h"
#include "Mesh.h"
#include "Refinement.h"
#include "Result.h"
#include "FileReader.h"
#include <iomanip>
#include <fstream>
#include <float.h>
#include <QLabel>
#include <QMenu>
#include <QThread>

#define DEFAULT_MONTE_CARLO (100)

Experiment::Experiment(SurfaceView *view)
{
	_passToResults = false;
	_mesh = NULL;
	_worker = NULL;
	_explorer = NULL;
	_data = NULL;
	_selected = NULL;
	_monteCount = -1;
	_dragging = false;
	_refinement = NULL;
	_view = view;
	_gl = NULL;
	_structure = NULL;
	_label = new QLabel("", _view);
	_label->setFocusProxy(_view);
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

void Experiment::recolourByCorrelation()
{
	if (!_refinement)
	{
		_refinement = new Refinement(this);
	}

	_refinement->recolourByScore();
}

void Experiment::meshStructure()
{
	Mesh *mesh = _structure->makeMesh();
	_gl->addObject(mesh, false);
	_mesh = mesh;

	refineMesh();
}

void Experiment::chooseTarget(Target t)
{
	Refinement::chooseTarget(t);
	_view->makeMenu();
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

bool Experiment::prepareWorkForMesh()
{
	if (_mesh == NULL)
	{
		return false;
	}

	if (_worker && _worker->isRunning())
	{
		return false;
	}
	
	if (!_worker)
	{
		_worker = new QThread();
	}

	_mesh->moveToThread(_worker);

	connect(_mesh, SIGNAL(resultReady()), this, SLOT(handleMesh()));
	return true;
}

void Experiment::smoothMesh()
{
	if (!prepareWorkForMesh())
	{
		return;
	}

	connect(this, SIGNAL(refine()), _mesh, SLOT(smoothCycles()));
	_worker->start();

	emit refine();
}

void Experiment::inflateMesh()
{
	if (!prepareWorkForMesh())
	{
		return;
	}

	connect(this, SIGNAL(refine()), _mesh, SLOT(inflateCycles()));
	_worker->start();

	emit refine();
}

void Experiment::removeMesh()
{
	if (_mesh == NULL)
	{
		return;
	}
	
	_structure->clearMesh();
	_mesh = NULL;
}

void Experiment::refineMesh()
{
	if (!prepareWorkForMesh())
	{
		return;
	}

	connect(this, SIGNAL(refine()), _mesh, SLOT(shrinkWrap()));
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
		QString name = QString::fromStdString(which->name());

		_view->convertToViewCoords(&x, &y);
		_label->setGeometry(x - 50, y - 50, 100, 50);
		_label->setText(name);
		_label->show();
		_view->setCursor(Qt::PointingHandCursor);
		
		if (_passToResults && _explorer)
		{
			_explorer->highlightBound(which);
		}
	}
	else
	{
		dehighlightAll();
		_view->setCursor(Qt::CrossCursor);

		if (_passToResults && _explorer)
		{
			_explorer->highlightBound(NULL);
		}
	}
}

void Experiment::dehighlightAll()
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		_bounds[i]->setHighlighted(false);
	}
	
	hideLabel();
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
	
	_view->setCursor(_dragging ? Qt::ClosedHandCursor : Qt::PointingHandCursor);
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

	_view->setCursor(Qt::OpenHandCursor);
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
	if (_data != NULL)
	{
		_data->changeFilename(filename);
		_data->load();
	}
	else
	{
		_data = new Data(filename);
	}

	createBinders();
}

void Experiment::createBinders()
{
	for (size_t i = 0; i < _data->boundCount(); i++)
	{
		std::string id = _data->boundName(i);
		
		if (_nameMap.count(id) > 0)
		{
			continue;
		}

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

void Experiment::svdRefine()
{
	refineModel(false, true);
}

void Experiment::refineModel(bool fixedOnly, bool svd)
{
	if (_worker && _worker->isRunning())
	{
		return;
	}
	
	if (!_worker)
	{
		_worker = new QThread();
	}

	_refinement = new Refinement(this);
	_refinement->moveToThread(_worker);

	_refinement->setFixedOnly(fixedOnly);
	_refinement->setConvert(svd);

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
	Refinement *obj = static_cast<Refinement *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), this, SLOT(handleResults()));
	disconnect(obj, SIGNAL(failed()), this, SLOT(handleError()));

	obj->deleteLater();
	_worker = NULL;
	
	if (_monteCount >= 0 && _monteCount < DEFAULT_MONTE_CARLO)
	{
		double newest = Refinement::getScore(_refinement);
		
		Result *result = new Result();
		result->savePositions(this);
		result->setScore(newest);
		_results.push_back(result);

		_monteCount++;

		monteCarloRound();
	}
	else if (_monteCount >= DEFAULT_MONTE_CARLO)
	{
		finishUpMonteCarlo();
	}
}

bool is_lesser_score(PosMapScore &a, PosMapScore &b)
{
	return (a.score < b.score);
}

void Experiment::finishUpMonteCarlo()
{
	_monteCount = -1;

	std::sort(_results.begin(), _results.end(), Result::result_is_less_than);

	for (size_t i = 0; i < 10 && i < _results.size(); i++)
	{
		std::cout << _results[i]->score() << std::endl;
	}

	if (_results.size())
	{
		std::cout << "Choosing best score: " << 
		_results[0]->score() << std::endl;

		_results[0]->applyPositions(this);
	}
	
	if (_explorer == NULL)
	{
		_explorer = new Explorer(NULL);
		_explorer->setExperiment(this);
		_gl->addObject(_explorer, false);
	}
	
	_explorer->addResults(_results);
	_results.clear();
	_explorer->show();
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
	for (int i = 0; i < 15; i++)
	{
		for (size_t i = 0; i < _bounds.size(); i++)
		{
			_bounds[i]->jiggleOnSurface(_structure);
		}
	}

}

void Experiment::monteCarloRound()
{
	std::cout << "Round " << _monteCount + 1 << ": " << std::flush;
	randomise();
	refineModel(true);
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
			name = &name[1];
			colour = true;
			std::cout << name << " marked in red" << std::endl;
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
		b->setSpecial(colour);

		if ((fixed && !b->isFixed()) ||
		    (!fixed && b->isFixed()))
		{
			b->toggleFixPosition();
		}
		
		b->colourFixed();

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
		std::string special = bi->isSpecial() ? "^" : "";
		
		posicsv << fixed << special << bin << "," << 
		posi.x << "," << -posi.y << "," << posi.z << std::endl;

		for (size_t j = 0; j < i; j++)
		{
			Bound *bj = bound(j);
			std::string bjn = bj->name();
			double val = _data->valueFor(bin, bjn);
			
			if (val != val)
			{
				continue;
			}

			double prop = bi->scoreWithOther(bj);
			
			file << bin << "," << bjn << "," << prop << std::endl;
		}
	}
	
	file.close();
	posicsv.close();
}


void Experiment::clearMonteCarlo()
{
	if (_explorer)
	{
		_explorer->clear();
	}

}
