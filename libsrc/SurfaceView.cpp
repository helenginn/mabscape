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

#define PAN_SENSITIVITY 30
#include "Refinement.h"
#include <iostream>
#include <ClusterList.h>
#include <FileReader.h>
#include <c4xsrc/AveCSV.h>
#include <c4xsrc/Group.h>
#include <Screen.h>
#include "SurfaceView.h"
#include <h3dsrc/SlipGL.h>
#include <h3dsrc/Dialogue.h>
#include "Genes.h"
#include "Structure.h"
#include "Refinement.h"
#include "Experiment.h"
#include "Controller.h"
#include "Bound.h"
#include <QTimer>
#include <QMenuBar>
#include <QThread>
#include <QLabel>

SurfaceView::SurfaceView(QWidget *p) : QMainWindow(p)
{
	_genes = new Genes();
	_screen = NULL;
	_mouseButton = Qt::NoButton;
	_controlPressed = false;
	_shiftPressed = false;
	_lastX = -2; _lastY = -2;
	_moving = false;
	_gl = new SlipGL(this);
	_gl->setZFar(2000);
	_experiment = new Experiment(this);
	_experiment->setGL(_gl);
	_gl->show();
	_gl->setAcceptsFocus(false);
	
	makeMenu();

	resize(600, 600);
	setMouseTracking(true);
	_gl->setMouseTracking(true);
}

void SurfaceView::startController(QThread *q, Controller *c)
{
	_experiment->makeExplorer();
	c->setView(this);
	c->moveToThread(q);
	connect(this, &SurfaceView::runController, c, &Controller::run);
	q->start();

	emit runController();
}

void SurfaceView::makeMenu()
{
	for (size_t i = 0; i < _menus.size(); i++)
	{
		_menus[i]->hide();
		_menus[i]->deleteLater();
	}

	for (size_t i = 0; i < _actions.size(); i++)
	{
		_actions[i]->deleteLater();
	}
	
	_menus.clear();
	_actions.clear();

	QMenu *structure = menuBar()->addMenu(tr("&Antigen"));
	_menus.push_back(structure);
	QAction *act = structure->addAction(tr("Load antigen surface"));
	connect(act, &QAction::triggered, this, &SurfaceView::loadSurface);
	_actions.push_back(act);
	act = structure->addAction(tr("Load antigen coordinates"));
	connect(act, &QAction::triggered, this, &SurfaceView::loadCoords);
	_actions.push_back(act);
	/*
	act = structure->addAction(tr("Triangulate structure"));
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::triangulateStructure);
	_actions.push_back(act);
	*/

	structure->addSeparator();
	act = structure->addAction(tr("Make collision mesh"));
	connect(act, &QAction::triggered, _experiment, &Experiment::meshStructure);
	_actions.push_back(act);
	act = structure->addAction(tr("Refine mesh"));
	connect(act, &QAction::triggered, _experiment, &Experiment::refineMesh);
	_actions.push_back(act);
	act = structure->addAction(tr("Triangulate mesh"));
	connect(act, &QAction::triggered, _experiment, &Experiment::triangulateMesh);
	act = structure->addAction(tr("Smooth mesh"));
	connect(act, &QAction::triggered, _experiment, &Experiment::smoothMesh);
	_actions.push_back(act);
	act = structure->addAction(tr("Inflate mesh"));
	connect(act, &QAction::triggered, _experiment, &Experiment::inflateMesh);
	_actions.push_back(act);
	act = structure->addAction(tr("Remove mesh"));
	connect(act, &QAction::triggered, _experiment, &Experiment::removeMesh);
	_actions.push_back(act);

	QMenu *data = menuBar()->addMenu(tr("&Data"));
	_menus.push_back(data);
	act = data->addAction(tr("Load competition &data"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::loadCSV);
	act = data->addAction(tr("Load binder &positions"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::loadPositions);
	act = data->addAction(tr("Launch cluster&4x"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::launchCluster4x);

	act = data->addAction(tr("Data to cluster4x"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::dataToCluster4x);
	act = data->addAction(tr("Model to cluster4x"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::modelToCluster4x);
	act = data->addAction(tr("Errors to cluster4x"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::errorsToCluster4x);

	_binders = menuBar()->addMenu(tr("&Antibodies"));
	_menus.push_back(_binders);
	
	_experiment->addBindersToMenu(_binders);
	
	act = _binders->addAction(tr("Write out antibody positions"));
	connect(act, &QAction::triggered, this, &SurfaceView::writeOutPositions);
	_actions.push_back(act);

	_binders->addSeparator();
	act = _binders->addAction(tr("Recolour by correlation"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::recolourByCorrelation);
	
	act = _binders->addAction(tr("Colour by CSV"));
	connect(act, &QAction::triggered, this, &SurfaceView::colourByCSV);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Identify non-competitors"));
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::identifyNonCompetitors);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Distance-competition values"));
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::plotDistanceCompetition);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Load genes"));
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::loadGenes);
	_actions.push_back(act);
	act = _binders->addAction(tr("Load sequences"));
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::loadSequences);
	_actions.push_back(act);

	QMenu *refine = menuBar()->addMenu(tr("&Refine"));
	_menus.push_back(refine);
	act = refine->addAction(tr("&Refine from here"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::unrestrainedRefine);

	QMenu *mc = refine->addMenu(tr("&Monte Carlo"));
	_menus.push_back(mc);

	act = mc->addAction(tr("Open results"));
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::openResults);
	_actions.push_back(act);
	act = mc->addAction(tr("Run &1000 cycles"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::monteCarlo);
	act = mc->addAction(tr("Start"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::mCarloStart);
	act = mc->addAction(tr("Stop"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::mCarloStop);

	act = refine->addAction(tr("Randomise positions"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::randomise);
	act = refine->addAction(tr("Jiggle"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::jiggle);
	refine->addSeparator();

	act = refine->addAction(tr("Use correlation"));
	_actions.push_back(act);
	act->setCheckable(true);
	act->setChecked(Refinement::currentTarget() == TargetCorrelation);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->chooseTarget(TargetCorrelation); });

	act = refine->addAction(tr("Use least squares"));
	_actions.push_back(act);
	act->setCheckable(true);
	act->setChecked(Refinement::currentTarget() == TargetLeastSquares);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->chooseTarget(TargetLeastSquares); });

	act = refine->addAction(tr("Use both"));
	_actions.push_back(act);
	act->setCheckable(true);
	act->setChecked(Refinement::currentTarget() == TargetBoth);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->chooseTarget(TargetBoth); });
	refine->addSeparator();

	act = refine->addAction(tr("Relocate flying antibodies"));
	_actions.push_back(act);
	act->setCheckable(true);
	bool relocate = Refinement::relocatingFliers();
	act->setChecked(relocate);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->relocateFliers(!relocate); });
}

void SurfaceView::convertCoords(double *x, double *y)
{
	double w = width();
	double h = height() - menuBar()->height();
	*y -= menuBar()->height();

	*x = 2 * *x / w - 1.0;
	*y =  - (2 * *y / h - 1.0);
}


void SurfaceView::convertToViewCoords(double *x, double *y)
{
	double w = width();
	double h = height();
	
	*x = (*x + 1.0) * w / 2;
	*y = (-*y + 1.0) * h / 2;
}

void SurfaceView::resizeEvent(QResizeEvent *event)
{
	_gl->setGeometry(0, menuBar()->height(), width(), 
	                 height() - menuBar()->height());
}

void SurfaceView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Alt || event->key() == Qt::Key_Control)
	{
		_controlPressed = true;
	}
	else if (event->key() == Qt::Key_Shift)
	{
		_shiftPressed = true;
	}
	else if (event->key() == Qt::Key_F)
	{
		_experiment->fixBound();
	}
	else if (event->key() == Qt::Key_L)
	{
		_experiment->fixLabel();
	}
}

void SurfaceView::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Alt || event->key() == Qt::Key_Control)
	{
		_controlPressed = false;
	}
	else if (event->key() == Qt::Key_Shift)
	{
		_shiftPressed = false;
	}
}

void SurfaceView::mousePressEvent(QMouseEvent *e)
{
	_lastX = e->x();
	_lastY = e->y();
	_mouseButton = e->button();
	_moving = false;
	
	_gl->restartTimer();
	
	if (_experiment->refinement())
	{
		_experiment->refinement()->pause(false);
	}
}

void SurfaceView::mouseMoveEvent(QMouseEvent *e)
{
	double x = e->x(); double y = e->y();
	convertCoords(&x, &y);

	if (_mouseButton == Qt::NoButton)
	{
		_experiment->hoverMouse(x, y, _shiftPressed);

		return;
	}
	
	if (_shiftPressed && !_moving)
	{
		/* we've begun moving with shift pressed - we must see
		 * if we have begun to drag a selected object */
	
		_experiment->checkDrag(x, y);
		_experiment->drag(x, y);
		_moving = true;
		return;
	}
	else if (_shiftPressed)
	{
		_experiment->drag(x, y);
		return;
	}
	
	_moving = true;
	_experiment->hideLabel();
	
	double newX = e->x();
	double xDiff = _lastX - newX;
	double newY = e->y();
	double yDiff = _lastY - newY;
	_lastX = newX;
	_lastY = newY;

	if (_mouseButton == Qt::LeftButton)
	{
		if (_controlPressed)
		{
			_gl->panned(xDiff / PAN_SENSITIVITY, yDiff / PAN_SENSITIVITY);
		}
		else
		{
			_gl->draggedLeftMouse(xDiff * 4, yDiff * 4);
		}
	}
	else if (_mouseButton == Qt::RightButton)
	{
		_gl->draggedRightMouse(xDiff * PAN_SENSITIVITY,
		                       yDiff * PAN_SENSITIVITY);
	}
	
	QList<QLabel *> ls = findChildren<QLabel *>("templabel");
	
	for (int i = 0; i < ls.size(); i++)
	{
		ls[i]->hide();
		ls[i]->deleteLater();
	}
}

void SurfaceView::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_moving && e->button() == Qt::LeftButton)
	{
		// this was just a click
		double x = e->x(); double y = e->y();
		convertCoords(&x, &y);
		_experiment->clickMouse(x, y);
	}
	
	if (_moving && e->button() == Qt::LeftButton && _shiftPressed)
	{
		_experiment->finishDragging();
	}
	
	_mouseButton = Qt::NoButton;
}

void SurfaceView::writeOutPositions()
{
	std::string filename = openDialogue(this, "Choose position CSV", 
	                                    "Comma-separated values (*.csv)",
	                                    true);
	_experiment->writeOutCSV(filename);
	
	makeMenu();
}

void SurfaceView::loadPositions()
{
	std::string filename = openDialogue(this, "Choose position CSV", 
	                                    "Comma-separated values (*.csv)");
	_experiment->loadPositions(filename);
	
	makeMenu();
}

void SurfaceView::loadCSV()
{
	std::string filename = openDialogue(this, "Choose competition data CSV", 
	                                    "Comma-separated values (*.csv)");
	_experiment->loadCSV(filename);
	
	makeMenu();
}

void SurfaceView::loadSurface()
{
	std::string filename = openDialogue(this, "Load surface file", 
	                                    "Wavefront object file (*.obj)");
	_experiment->loadStructure(filename);
}

void SurfaceView::loadCoords()
{
	std::string filename = openDialogue(this, "Load structure coordinates", 
	                                    "Protein data bank file (*.pdb)");
	_experiment->loadStructureCoords(filename);
}

void SurfaceView::unrestrainedRefine()
{
	_experiment->refineFurther();
}

void SurfaceView::fixToSurfaceRefine()
{
	_experiment->refineModel(true);
}


void SurfaceView::launchCluster4x()
{
	if (_screen != NULL)
	{
		_screen->show();
	}

	_screen = new Screen(NULL);
	_screen->setWindowTitle("cluster4x - abmap");
	ClusterList *list = _screen->getList();
	AveCSV *csv = _experiment->csv();
	csv->setList(list);

	csv->startNewCSV("Model");
	_experiment->updateCSV(csv, 0);
	csv->startNewCSV("Errors");
	_experiment->updateCSV(csv, 2);

	csv->preparePaths();
	list->addCSVSwitcher();
	csv->setChosen(0);

	Group *top = Group::topGroup();
	top->setCustomName("Data, model, errors");
	top->updateText();

	_screen->show();
	makeMenu();
}

void SurfaceView::modelToCluster4x()
{
	_experiment->somethingToCluster4x(0);
}

void SurfaceView::dataToCluster4x()
{
	_experiment->somethingToCluster4x(1);
}

void SurfaceView::errorsToCluster4x()
{
	_experiment->somethingToCluster4x(2);
}

void SurfaceView::colourByCSV()
{
	std::string filename = openDialogue(this, "Choose colour CSV", 
	                                    "Comma-separated values (*.csv)");
	
	if (filename.length() == 0)
	{
		return;
	}

	_experiment->recolourByCSV(filename);

}

void SurfaceView::identifyNonCompetitors()
{
	std::string filename = openDialogue(this, "Choose comma-separated antibodies", 
	                                    "Comma-separated values (*.csv)");
	if (filename.length() == 0)
	{
		return;
	}

	std::string contents = get_file_contents(filename);
	std::vector<std::string> abs = split(contents, ',');

	_experiment->findNonCompetitors(abs);
}

void SurfaceView::plotDistanceCompetition()
{
	_experiment->plotDistanceCompetition();

}

void SurfaceView::pause()
{
	_gl->pause();
	if (_experiment->refinement())
	{
		_experiment->refinement()->pause(true);
	}
}

void SurfaceView::loadGenes()
{
	std::string filename = openDialogue(this, "Choose formatted genes", 
	                                    "Comma-separated values (*.csv)");
	if (filename.length() == 0)
	{
		return;
	}

	_genes->loadData(filename);
}

void SurfaceView::loadSequences()
{
	std::string filename = openDialogue(this, "Choose formatted genes", 
	                                    "Comma-separated values (*.csv)");
	if (filename.length() == 0)
	{
		return;
	}

	_genes->loadSequences(filename);
}
