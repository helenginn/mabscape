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
#include "Experiment.h"
#include "PDBView.h"
#include "Explorer.h"
#include "SurfaceView.h"
#include <h3dsrc/SlipGL.h>
#include "Bound.h"
#include "Data.h"
#include "Structure.h"
#include <h3dsrc/Mesh.h>
#include "Refinement.h"
#include "Result.h"
#include <hcsrc/FileReader.h>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <c4xsrc/AveCSV.h>
#include <c4xsrc/Group.h>
#include <c4xsrc/ClusterList.h>
#include <hcsrc/mat4x4.h>
#include <libsrc/Absolute.h>
#include <libsrc/Atom.h>
#include <libsrc/Molecule.h>
#include <libsrc/Crystal.h>
#include <libsrc/Options.h>
#include <Screen.h>
#include <float.h>
#include <QLabel>
#include <QMenu>
#include <QThread>
#include <QVariant>

#define DEFAULT_MONTE_CARLO (1000)

Experiment::Experiment(SurfaceView *view)
{
	_labels = false;
	_passToResults = false;
	_mesh = NULL;
	_worker = NULL;
	_explorer = NULL;
	_data = NULL;
	_selected = NULL;
	_monteCount = -1;
	_monteTarget = DEFAULT_MONTE_CARLO;
	_dragging = false;
	_refinement = new Refinement(this);
	_view = view;
	_winsx = -2;
	_winsy = -2;
	_winex = -2;
	_winey = -2;
	_windowing = false;
	_selWindow = new QLabel("", _view);
	_selWindow->setStyleSheet("QLabel { border: 1px solid white; background-color: transparent; }");
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
	if (_structure != NULL)
	{
		std::cout << "Got structure already, not loading new one." 
		<< std::endl;
		return;
	}

	Structure *str = new Structure(filename);
	_gl->addObject(str, true);
	_structure = str;

	emit alteredMenu();
}

void Experiment::loadStructureCoords(std::string filename)
{
	if (_structure == NULL)
	{
		std::cout << "No structure." << std::endl;
		return;
	}
	
	prepareWorkForObject(_structure);
	connect(_structure, SIGNAL(resultReady()), this, SLOT(handleMesh()));
	connect(_structure, SIGNAL(resultReady()), _view, SLOT(makeMenu()));
	connect(this, &Experiment::refine, _structure, 
	        [=]() { _structure->addPDB(filename);} );
	
	emit refine();
}

void Experiment::recolourByCorrelation()
{
	if (!_refinement)
	{
		_refinement = new Refinement(this);
	}

	_refinement->recolourByScore();
}

QThread *Experiment::runSmoothMesh()
{
	std::cout << "Smoothing mesh..." << std::endl;
	if (_mesh == NULL)
	{
		return _worker;
	}

	smoothMesh();
	return _worker;
}

QThread *Experiment::runInflateMesh()
{
	if (_mesh == NULL)
	{
		return _worker;
	}

	std::cout << "Inflating mesh" << std::endl;
	inflateMesh();
	return _worker;
}

void Experiment::meshStructure()
{
	if (_mesh == NULL)
	{
		Mesh *mesh = _structure->makeMesh();
		_gl->addObject(mesh, false);
		_mesh = mesh;
	}
}

void Experiment::relocateFliers(bool relocate)
{
	Refinement::setRelocateFliers(relocate);
	emit alteredMenu();
}

void Experiment::chooseTarget(Target t)
{
	Refinement::chooseTarget(t);
	emit alteredMenu();
}

void Experiment::triangulateStructure()
{
	if (_structure && !isSomethingHappening())
	{
		_structure->triangulate();
	}
}

void Experiment::triangulateMesh()
{
	if (_mesh == NULL || isSomethingHappening())
	{
		return;
	}

	_mesh->changeToTriangles();
	_mesh->SlipObject::triangulate();
	_mesh->changeToLines();
}

bool Experiment::isSomethingHappening()
{
	return (_worker && _worker->isRunning());
}

bool Experiment::prepareWorkForObject(QObject *object)
{
	if (object == NULL)
	{
		return false;
	}

	if (_worker && _worker->isRunning())
	{
		std::cout << "Waiting for worker to finish old job" << std::endl;
		_worker->wait();
	}
	
	if (!_worker)
	{
		_worker = new QThread();
	}

	object->moveToThread(_worker);

	return true;
}

bool Experiment::prepareWorkForMesh()
{
	bool success = prepareWorkForObject(_mesh);
	
	if (!success)
	{
		return false;
	}

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
	if (_mesh == NULL || isSomethingHappening())
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


Bound *Experiment::loadBound()
{
	Bound *bnd = new Bound();
	bnd->setName("thing" + i_to_str(_bounds.size() + 1));
	_gl->addObject(bnd, false);
	
	if (_structure)
	{
		bnd->randomlyPositionInRegion(_structure);
		bnd->snapToObject(_structure);
		bnd->setStructure(_structure);
	}

	_gl->removeObject(_explorer);
	_gl->addObject(_explorer, false);

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

void Experiment::hoverMouse(double x, double y, bool shift)
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
		
		if (_passToResults && _explorer && !shift)
		{
			_explorer->highlightBound(which);
		}
	}
	else
	{
		dehighlightAll();
		_view->setCursor(Qt::CrossCursor);

		if (_passToResults && _explorer && !shift)
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
		_bounds[i]->selectMe(false);
	}
	
	_selected = NULL;
	hideLabel();
}

void Experiment::select(Bound *bound, double x, double y)
{
	_selected = bound;
	bound->selectMe(true);
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
	}
	else
	{
		double z = -FLT_MAX;
		_dragging = _selected->intersects(x, y, &z);
	}

	_view->setCursor(_dragging ? Qt::ClosedHandCursor : 
	                 Qt::PointingHandCursor);
	
	if (!_dragging)
	{
		_windowing = true;
	}
}

void Experiment::updateWindow()
{
	double x1 = _winsx;
	double y1 = _winsy;
	double x2 = _winex;
	double y2 = _winey;
	_view->convertToViewCoords(&x1, &y1);
	_view->convertToViewCoords(&x2, &y2);
	_selWindow->setGeometry(x1, y1, (x2 - x1), (y2 - y1));
	_selWindow->show();
}

void Experiment::drawWindow(double x, double y)
{
	if (_winsx < -1 && _winsy < -1)
	{
		_winsx = x;
		_winsy = y;
		_winex = x;
		_winey = y;
	}
	else
	{
		_winex = x;
		_winey = y;
	}

	updateWindow();
}

void Experiment::drag(double x, double y)
{
	if (_windowing)
	{
		drawWindow(x, y);
	}

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
	_windowing = false;

	if (_selected)
	{
		_selected->snapToObject(_structure);
	}
	
	if (_winsx >= -1 && _winsy >= -1)
	{
		if (_explorer)
		{
			_explorer->selectSubset(_winsx, _winsy, _winex, _winey);
		}
		_selWindow->hide();
		_winsx = -2;
		_winsy = -2;
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

	emit alteredMenu();
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
		Bound *bound = loadBound();
		bound->setName(id);
		_nameMap[id] = bound;
	}
}

void Experiment::addBindersToMenu()
{
	QMenu *m = _view->getBinderMenu();
	addBindersToMenu(m);
}

void Experiment::addBindersToMenu(QMenu *binders)
{
	QList<QAction *> actions = binders->findChildren<QAction *>("binder");
	for (int i = 0; i < actions.size(); i++)
	{
		binders->removeAction(actions[i]);
	}

	QList<QMenu *> menus = binders->findChildren<QMenu *>("binder");
	for (int i = 0; i < menus.size(); i++)
	{
		QAction *menuToBeRemoved = menus[i]->menuAction();
		binders->removeAction(menuToBeRemoved);
	}

	for (size_t i = 0; i < _bounds.size(); i += 10)
	{
		size_t end = std::min(i + 10, _bounds.size());

		std::string title = "Binders " + i_to_str(i + 1) + " to " 
		+ i_to_str(end);

		QMenu *m = binders->addMenu(QString::fromStdString(title));
		m->setObjectName("binder");

		for (size_t j = 0; i + j < _bounds.size() && j < 10; j++)
		{
			void *b = &*_bounds[i + j];
			std::string name = _bounds[i + j]->name();
			QString n = QString::fromStdString(name);
			QMenu *mb = m->addMenu(n);
			mb->setObjectName("binder");

			QVariant v = QVariant::fromValue(b);
			QAction *act = mb->addAction("Select");
			act->setObjectName("binder");
			act->setProperty("bound", v);
			connect(act, &QAction::triggered, 
			        this, &Experiment::selectFromMenu);

			act = mb->addAction("Fix from PDB");
			act->setObjectName("binder");
			act->setProperty("bound", v);
			connect(act, &QAction::triggered, 
			        this, &Experiment::fixFromPDB);
		}
	}
}

void Experiment::selectFromMenu()
{
	QObject *obj = QObject::sender();
	QAction *act = static_cast<QAction *>(obj);

	void *v = act->property("bound").value<void *>();
	Bound *b = static_cast<Bound *>(v);

	if (b == NULL)
	{
		return;
	}

	deselectAll();
	select(b, -1, -1);
}

void Experiment::refineFurther()
{
	if (_worker && _worker->isRunning())
	{
		std::cout << "Doing something else" << std::endl;
		return;
	}
	
	if (!_worker)
	{
		_worker = new QThread();
	}

	_refinement = new Refinement(this);
	_refinement->setRandomiseFirst(false);
	_refinement->moveToThread(_worker);

	connect(this, SIGNAL(refine()),
	        _refinement, SLOT(refine()));

	connect(_refinement, SIGNAL(resultReady()), 
	        this, SLOT(oneTimeResults()));
	connect(_refinement, SIGNAL(failed()), 
	        this, SLOT(handleError()));
	_worker->start();

	emit refine();
}

void Experiment::refineModel(bool fixedOnly, bool svd)
{
	if (_worker && _worker->isRunning() && !fixedOnly)
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

	connect(this, SIGNAL(refine()),
	        _refinement, SLOT(refine()));

	connect(_refinement, SIGNAL(resultReady()), 
	        this, SLOT(handleResults()));
	connect(_refinement, SIGNAL(failed()), 
	        this, SLOT(handleError()));
	_worker->start();

	emit refine();
}

bool Experiment::isRunningMonteCarlo()
{
	int count = 0;
	_mut.lock();
	count = _monteCount + 1;
	_mut.unlock();

	bool finished = (count >= _monteTarget);

	return !finished;
}

void Experiment::oneTimeResults()
{
	Refinement *obj = static_cast<Refinement *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), nullptr, nullptr);
	disconnect(obj, SIGNAL(failed()), this, SLOT(handleError()));

	double newest = Refinement::getScore(obj);

	Result *result = new Result();
	result->setExperiment(this);
	result->savePositions();
	result->setScore(fabs(newest));
	_results.push_back(result);
	_explorer->addResults(_results);
	_results.clear();

	obj->deleteLater();
	_worker->quit();
	_worker->wait();
}

void Experiment::handleResults()
{
	Refinement *obj = static_cast<Refinement *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), this, SLOT(handleResults()));
	disconnect(obj, SIGNAL(failed()), this, SLOT(handleError()));

	bool finished = !isRunningMonteCarlo();

	double newest = Refinement::getScore(obj);

	Result *result = new Result();
	result->setExperiment(this);
	result->savePositions();
	result->setScore(newest);
	_results.push_back(result);
	_explorer->addResults(_results);
	_results.clear();

	if (!finished)
	{
		if (_monteCount >= 0)
		{
			_mut.lock();
			_monteCount++;
			_mut.unlock();
		}

		monteCarloRound();
	}
	else 
	{
		std::cout << "Finishing Monte Carlo" << std::endl;
		finishUpMonteCarlo();
	}

	obj->deleteLater();
}

bool is_lesser_score(PosMapScore &a, PosMapScore &b)
{
	return (a.score < b.score);
}

void Experiment::makeExplorer()
{
	if (_explorer == NULL)
	{
		_explorer = new Explorer(NULL);
		_explorer->setExperiment(this);
		_explorer->setClusterScreen(_view->clusterScreen());
		_gl->addObject(_explorer, false);
	}

}

void Experiment::finishUpMonteCarlo()
{
	_monteCount = -1;

	std::sort(_results.begin(), _results.end(), Result::result_is_less_than);

	makeExplorer();
	_explorer->addResults(_results);
	_results.clear();
	_explorer->show();

	_worker->quit();
	_worker->wait();
}

void Experiment::handleMesh()
{
	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(nullptr, nullptr, this, SLOT(handleMesh()));

	_worker->quit();
//	_worker->wait();

	emit alteredMenu();
}

void Experiment::handleError()
{
	Refinement *obj = static_cast<Refinement *>(QObject::sender());

	disconnect(this, SIGNAL(refine()), nullptr, nullptr);
	disconnect(obj, SIGNAL(resultReady()), this, SLOT(handleResults()));
	disconnect(obj, SIGNAL(failed()), this, SLOT(handleError()));

	obj->deleteLater();
	_worker->quit();
	_worker->wait();
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
	std::string contents;
	try
	{
		contents = get_file_contents(filename);
	}
	catch (int e)
	{
		std::cout << "Could not find file" << std::endl;
		return;
	}
	
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

		float v1 = atof(bits[1].c_str());
		float v2 = atof(bits[2].c_str());
		float v3 = atof(bits[3].c_str());

		vec3 pos = make_vec3(v1, v2, v3);
		Bound *b = _nameMap[name];
		
		if (b == NULL)
		{
			std::cout << "Couldn't find " << name << std::endl;
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
		b->snapToObject(NULL);
	}

}

void Experiment::mCarloStop()
{
	_mut.lock();
	_monteCount = _monteTarget;
	_mut.unlock();
}

void Experiment::mCarloStart()
{
	_monteCount = -2;

	monteCarloRound();
}

QThread *Experiment::monteCarlo()
{
	_monteCount = 0;

	monteCarloRound();
	return _worker;
}

void Experiment::writeOutCSV(std::string filename)
{
	std::ofstream posicsv;
	posicsv.open(filename); 
	std::ofstream file;
	file.open("model.csv"); 

	for (size_t i = 0; i < boundCount(); i++)
	{
		Bound *bi = bound(i);
		std::string bin = bi->name();
		vec3 posi = bi->getStoredPosition();
		std::string fixed = bi->isFixed() ? "*" : "";
		std::string special = bi->isSpecial() ? "^" : "";
		
		posicsv << fixed << special << bin << "," << 
		posi.x << "," << posi.y << "," << posi.z << std::endl;

		for (size_t j = 0; j < i; j++)
		{
			Bound *bj = bound(j);
			std::string bjn = bj->name();
			double val = _data->valueFor(bin, bjn);
			
			if (val != val)
			{
//				continue;
			}

			double prop = bi->scoreWithOther(bj);
			
			file << bin << "," << bjn << "," << prop << std::endl;
		}
	}
	
	file.close();
	posicsv.close();
}

AveCSV *Experiment::csv()
{
	if (_explorer)
	{
		_explorer->setClusterScreen(_view->clusterScreen());
	}

	return _data->getClusterCSV();
}

void Experiment::updateCSV(AveCSV *csv, int data)
{
	if (data == 1)
	{
		_data->updateCSV(csv);
		return;
	}

	for (size_t i = 0; i < boundCount(); i++)
	{
		Bound *bi = bound(i);
		std::string bin = bi->name();

		for (size_t j = 0; j < i; j++)
		{
			Bound *bj = bound(j);
			std::string bjn = bj->name();
			double val = _data->valueFor(bin, bjn);
			if (val != val && data != 0)
			{
				continue;
			}

			double prop = bi->scoreWithOther(bj);
			
			if (data == 2)
			{
				prop -= val;
				prop = fabs(prop);
			}
			
			csv->addValue(bin, bjn, prop);
			csv->addValue(bjn, bin, prop);
		}
	}
}

void Experiment::somethingToCluster4x(int data)
{
	if (_view->clusterScreen() == NULL)
	{
		_view->launchCluster4x();
	}

	ClusterList *list = _view->clusterScreen()->getList();
	size_t count = list->groupCount();
	Group *grp = list->group(count - 1);
	AveCSV *csv = Group::topGroup()->getAveCSV();
	if (data == 0)
	{
		AveCSV::setChosen("Model");
	}
	else if (data == 1)
	{
		AveCSV::setChosen("Data");
	}
	else if (data == 2)
	{
		AveCSV::setChosen("Errors");
	}

	updateCSV(csv, data);
	_view->clusterScreen()->getList()->cluster(grp);
}


void Experiment::openResults()
{
	makeExplorer();
	_explorer->show();
}

void Experiment::fixFromPDB()
{
	QObject *obj = QObject::sender();
	QAction *act = static_cast<QAction *>(obj);

	void *v = act->property("bound").value<void *>();
	Bound *b = static_cast<Bound *>(v);
	PDBView *view = new PDBView(b);

	if (!view->successful())
	{
		view->deleteLater();
	}
	else
	{
		view->setExperiment(this);
	}
}


void Experiment::fixLabel()
{
	QLabel *l = new QLabel(_label->text(), _view);
	l->setGeometry(_label->geometry());
	l->setFocusProxy(_view);
	l->setMouseTracking(true);
	l->setObjectName("templabel");
	l->setStyleSheet(_label->styleSheet());
	QFont font = QFont("Helvetica", 64);
	l->setFont(font);
	l->setAlignment(Qt::AlignVCenter);
	l->setAlignment(Qt::AlignHCenter);
	l->show();
}

void Experiment::addNonCompetitor(std::vector<std::string> abs,
                                  std::vector<std::string> &chain,
                                  std::vector<std::string> *addable)
{
	std::vector<std::string>::iterator loc;
	for (size_t i = 0; i < abs.size(); i++)
	{
		loc = std::find(chain.begin(), chain.end(), abs[i]);
		
		if (loc != chain.end())
		{
			continue;
		}
		
		bool ok = true;
		for (size_t j = 0; j < chain.size(); j++)
		{
			double comp = _data->valueFor(abs[i], chain[j]);
			if (comp != comp)
			{
				ok = false;
			}

			if (comp > 0.2)
			{
				ok = false;
			}
		}
		
		if (ok)
		{
			addable->push_back(abs[i]);
		}
	}
}

void Experiment::findNonCompetitors(std::vector<std::string> abs)
{
	for (size_t i = 0; i < abs.size(); i++)
	{
		if (bound(abs[i]) == NULL)
		{
			abs.erase(abs.begin() + i);
			i--;
		}
	}

	for (size_t i = 0; i < abs.size(); i++)
	{
		std::vector<std::string> chain;
		chain.push_back(abs[i]);

		std::vector<std::vector<std::string> > extendables;
		extendables.push_back(chain);

		size_t num = 0;
		
		while (num < extendables.size())
		{
			std::vector<std::string> addable;
			addNonCompetitor(abs, extendables[num], &addable);
			
			for (size_t j = 0; j < addable.size(); j++)
			{
				std::vector<std::string> newchain;
				newchain.reserve(extendables[num].size() + 1);
				newchain.insert(newchain.begin(), extendables[num].begin(),
				                extendables[num].end());
				newchain.push_back(addable[j]);
				extendables.push_back(newchain);
			}
			
			if (addable.size() == 0 && extendables[num].size() > 1)
			{
				std::sort(extendables[num].begin(), extendables[num].end(), 
				          std::less<std::string>());

				for (size_t j = 0; j < extendables[num].size(); j++)
				{
					std::cout << extendables[num][j] << " " << std::flush;
				}

				std::cout << std::endl;
			}

			num++;
		}
	}
}

void Experiment::plotDistanceCompetition()
{
	std::ofstream file;
	file.open("distance_competition.csv");
	
	file << "first, second, distance, data, model" << std::endl;

	for (size_t i = 0; i < boundCount() - 1; i++)
	{
		Bound *bi = bound(i);
		vec3 posi = bi->getWorkingPosition();

		for (size_t j = i; j < boundCount(); j++)
		{
			Bound *bj = bound(j);
			
			double val = _data->valueFor(bi->name(), bj->name());
			
			if (val != val)
			{
				continue;
			}

			vec3 posj = bj->getWorkingPosition();
			vec3 diff = vec3_subtract_vec3(posi, posj);
			
			double l = vec3_length(diff);

			double score = bi->scoreWithOther(bj);

			file << bi->name() << ", " << bj->name() << ", ";
			file << l << ", " << val << ", " << score << std::endl;
		}
	}

	file.close();
}

QThread *Experiment::worker()
{
	if (!_worker)
	{
		_worker = new QThread();
	}

	return _worker;
}

void Experiment::recolourByBoundErrors(double data, double model)
{
	double mean = (data + model) / 2;
	for (size_t i = 0; i < boundCount(); i++)
	{
		Bound *b = bound(i);
		
		if (b == _selected)
		{
			b->recolour(1, 1, 0);
			continue;
		}
		
		double pred = _selected->scoreWithOther(b);
		double obs = _data->valueFor(b->name(), _selected->name());
		
		double diff = obs * data + pred * model;
		b->setValue(diff);
		b->colourByValue(mean, 0.5);
	}
}

void Experiment::writePDB(std::string filename)
{
	CrystalPtr crystal = CrystalPtr(new Vagabond::Crystal());
	MoleculePtr mol = MoleculePtr(new Molecule());
	mol->setChainID("V");
	crystal->addMolecule(mol);
	Options::getRuntimeOptions()->addCrystal(crystal);
	std::map<Bound *, double> _map;

	std::ofstream f;
	f.open(filename);
	double sqSum = 0;
	double count = 0;

	std::ostringstream header;
	
	Metadata *m = metadata();
	
	for (size_t j = 0; j < boundCount(); j++)
	{
		Bound *b = bound(j);

		double rmsd = b->RMSD();
		
		if (rmsd != rmsd)
		{
			rmsd = 0;
		}
		
		double val = NAN;
		if (b->getValue() == b->getValue())
		{
			val = b->getValue();
		}
		
		/*
		if (val != val)
		{
			val = rmsd;
		}
		*/

		vec3 mean = b->getStoredPosition();

		AbsolutePtr abs = AbsolutePtr(new Absolute(mean, val, "HG", rmsd));
		
		int num = 0;
		std::string name = b->name();
		
		for (size_t i = 0; i < name.length(); i++)
		{
			if (name[i] >= '0' && name[i] <= '9')
			{
				char *n = &name[i];
				num = atoi(n);
				break;
			}
		}
		
		std::string chain = "V";
		
		if (m->hasKey(b, "chain"))
		{
			chain = m->valueForKey(b, "chain");
		}

		abs->setIdentity(num, chain, "ABS", "AB", num);
		abs->addToMolecule(mol);
		
		header << "REMARK " << b->name() << " IS AB    " << 
		abs->getAtom()->getAtomNum() << std::endl;
		
		if (b->isFixed())
		{
			continue;
		}
		
		b->setRealPosition(mean);
		b->snapToObject(NULL);
		b->updatePositionToReal();
		
		sqSum += rmsd * rmsd;
		_map[b] = rmsd;
		count++;
	}
	
	header << "REMARK" << std::endl;
	
	f << header.str() << std::endl;

	std::string str = mol->makePDB(PDBTypeAverage, CrystalPtr(), -1);
	f << str;
	f.close();
	
}

void Experiment::toggleLabels()
{
	_labels = !_labels;

	for (size_t i = 0; i < boundCount(); i++)
	{
		bound(i)->label(_labels);
	}
}

bool Experiment::loadedSequences()
{
	return _view->genes()->antibodyCount() > 0;
}

void Experiment::recolourBySequence(bool heavy)
{
	for (size_t i = 0; i < boundCount(); i++)
	{
		Bound *b = bound(i);
		
		if (b == _selected)
		{
			b->recolourBoth(1, 1, 0);
			continue;
		}
		
		double value = _view->genes()->valueFor(heavy, b, _selected);
		
		if (value != value)
		{
			b->setValue(NAN);
		}
		
		b->setValue(value);
		b->colourByValue(0.5, 0.5);
	}
}

void Experiment::makeNamedBound(std::string name)
{
	std::cout << "Making antibody named " << name << std::endl;
	Bound *bnd = loadBound();
	bnd->setName(name);
	_nameMap[name] = bnd;
	std::cout << _bounds.size() << std::endl;
	emit alteredMenu();
}
