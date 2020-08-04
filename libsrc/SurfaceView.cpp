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
#include "SurfaceView.h"
#include "SlipGL.h"
#include "Structure.h"
#include "Experiment.h"
#include "Bound.h"
#include <QTimer>
#include <QMenuBar>
#include <QFileDialog>

SurfaceView::SurfaceView(QWidget *p) : QMainWindow(p)
{
	_mouseButton = Qt::NoButton;
	_controlPressed = false;
	_shiftPressed = false;
	_lastX = -1; _lastY = -1;
	_moving = false;
	_gl = new SlipGL(this);
	_experiment = new Experiment(this);
	_experiment->setGL(_gl);
	_gl->show();
	
	makeMenu();

	resize(1000, 1000);
	setFocus();
	setMouseTracking(true);
	_gl->setMouseTracking(true);
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

	QMenu *structure = menuBar()->addMenu(tr("&Structure"));
	_menus.push_back(structure);
	QAction *act = structure->addAction(tr("Make collision mesh"));
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
	act = data->addAction(tr("Load competition data"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::loadCSV);
	act = data->addAction(tr("Load positions"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::loadPositions);

	QMenu *binders = menuBar()->addMenu(tr("&Binders"));
	_menus.push_back(binders);
	
	_experiment->addBindersToMenu(binders);
	
	act = binders->addAction(tr("Write CSV"));
	connect(act, &QAction::triggered, _experiment, &Experiment::writeOutCSV);
	_actions.push_back(act);

	QMenu *refine = menuBar()->addMenu(tr("&Refine"));
	_menus.push_back(refine);
	act = refine->addAction(tr("Refine"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::unrestrainedRefine);
	act = refine->addAction(tr("Refine, axis change"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::svdRefine);
	act = refine->addAction(tr("Monte Carlo"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::monteCarlo);
	act = refine->addAction(tr("Clear results"));
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::clearMonteCarlo);
	_actions.push_back(act);
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

	refine->addSeparator();
	act = refine->addAction(tr("Recolour by correlation"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::recolourByCorrelation);
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
}


void SurfaceView::mouseMoveEvent(QMouseEvent *e)
{
	double x = e->x(); double y = e->y();
	convertCoords(&x, &y);

	if (_mouseButton == Qt::NoButton)
	{
		_experiment->hoverMouse(x, y);

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
			_gl->draggedLeftMouse(-xDiff * 4, -yDiff * 4);
		}
	}
	else if (_mouseButton == Qt::RightButton)
	{
		_gl->draggedRightMouse(xDiff * PAN_SENSITIVITY,
		                       yDiff * PAN_SENSITIVITY);
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

void SurfaceView::loadPositions()
{
	QFileDialog *f = new QFileDialog(this, "Choose position CSV", 
	                                 "Comma-separated values (*.csv)");
	f->setFileMode(QFileDialog::AnyFile);
	f->setOptions(QFileDialog::DontUseNativeDialog);
	f->show();

    QStringList fileNames;

    if (f->exec())
    {
        fileNames = f->selectedFiles();
    }
    
    if (fileNames.size() < 1)
    {
		return;
    }

	f->deleteLater();
	std::string filename = fileNames[0].toStdString();
	
	_experiment->loadPositions(filename);
	
	makeMenu();
}

void SurfaceView::loadCSV()
{
	QFileDialog *f = new QFileDialog(this, "Choose competition data CSV", 
	                                 "Comma-separated values (*.csv)");
	f->setFileMode(QFileDialog::AnyFile);
	f->setOptions(QFileDialog::DontUseNativeDialog);
	f->show();

    QStringList fileNames;

    if (f->exec())
    {
        fileNames = f->selectedFiles();
    }
    
    if (fileNames.size() < 1)
    {
		return;
    }

	f->deleteLater();
	std::string filename = fileNames[0].toStdString();
	
	_experiment->loadCSV(filename);
	
	makeMenu();
}

void SurfaceView::unrestrainedRefine()
{
	_experiment->refineModel(false);
}

void SurfaceView::fixToSurfaceRefine()
{
	_experiment->refineModel(true);
}

