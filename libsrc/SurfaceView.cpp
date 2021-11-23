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
#include "Metadata.h"
#include <iostream>
#include <hcsrc/FileReader.h>
#include <c4xsrc/ClusterList.h>
#include <c4xsrc/AveCSV.h>
#include <c4xsrc/Group.h>
#include <c4xsrc/Screen.h>
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
#include <QMessageBox>

SurfaceView::SurfaceView(QWidget *p) : QMainWindow(p)
{
	_metadata = new Metadata();
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
	_experiment->setMetadata(_metadata);
	connect(_experiment, &Experiment::alteredMenu,
	        this, &SurfaceView::updateMenu);
	_gl->show();
	_gl->setAcceptsFocus(false);
	
	makeMenu();
	
	setFocusPolicy(Qt::StrongFocus);
	resize(600, 600);
	setMouseTracking(true);
	_gl->setMouseTracking(true);
}
/* making sure that all windows are closed upon closing the SurfaceView and 
that all progress is saved */

void SurfaceView::closeEvent(QCloseEvent *event)
{
	QMessageBox exit_check;
	exit_check.setText("Are you sure you want to quit the Programm?");
	exit_check.setInformativeText("Any unsaved progress will be lost.");
	exit_check.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
	exit_check.setDefaultButton(QMessageBox::Cancel);
	int check = exit_check.exec();
	
	if (check == QMessageBox::Ok)
	{
		QApplication::quit();
	}
	else
	{
		event->ignore();
	}
}

void SurfaceView::focusInEvent(QFocusEvent *e)
{
	_controlPressed = false;
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

void SurfaceView::updateMenu()
{
	bool no_structure = (_experiment->structure() == NULL);
	bool no_data = (_experiment->boundCount() == 0);
	bool no_residues = (no_structure 
	                    || _experiment->structure()->hasResidues());

	QMenuBar *b = menuBar();
	QAction *act = b->findChild<QAction *>("highlight_residues");
	act->setDisabled(no_residues);


	QList<QAction *> widgets = b->findChildren<QAction *>("need_structure");
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->setDisabled(no_structure);
	}

	widgets = b->findChildren<QAction *>("need_data");
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->setDisabled(no_structure || no_data);
	}

	QMenu *menu = b->findChild<QMenu *>("metadata");
	if (menu)
	{
		menu->setDisabled(no_structure || no_data);
		_metadata->makeMenu(menu, _experiment);
	}
	
	act = b->findChild<QAction *>("correlation");
	act->setChecked(Refinement::currentTarget() == TargetCorrelation);

	act = b->findChild<QAction *>("least_squares");
	act->setChecked(Refinement::currentTarget() == TargetLeastSquares);

	act = b->findChild<QAction *>("both");
	act->setChecked(Refinement::currentTarget() == TargetBoth);

	act = b->findChild<QAction *>("relocate");
	bool relocate = Refinement::relocatingFliers();
	act->setChecked(relocate);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->relocateFliers(!relocate); });
	
	if (!no_data)
	{
		_experiment->addBindersToMenu();
	}
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
	
	act = structure->addAction(tr("Triangulate structure"));
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::triangulateStructure);
	_actions.push_back(act);
	
	act = structure->addAction(tr("Highlight residues"));
	act->setObjectName("highlight_residues");
	connect(act, &QAction::triggered, this, 
	        &SurfaceView::highlightResidues);
	if (!_experiment->structure() ||
	    !_experiment->structure()->hasResidues())
	{
		act->setDisabled(true);
	}
	_actions.push_back(act);

	structure->addSeparator();
	act = structure->addAction(tr("Make collision mesh"));
	act->setObjectName("need_structure");
	connect(act, &QAction::triggered, _experiment, &Experiment::meshStructure);
	_actions.push_back(act);
	act = structure->addAction(tr("Refine mesh"));
	act->setObjectName("need_structure");
	connect(act, &QAction::triggered, _experiment, &Experiment::refineMesh);
	_actions.push_back(act);
	act = structure->addAction(tr("Triangulate mesh"));
	act->setObjectName("need_structure");
	connect(act, &QAction::triggered, _experiment, &Experiment::triangulateMesh);
	act = structure->addAction(tr("Smooth mesh"));
	act->setObjectName("need_structure");
	connect(act, &QAction::triggered, _experiment, &Experiment::smoothMesh);
	_actions.push_back(act);
	act = structure->addAction(tr("Inflate mesh"));
	act->setObjectName("need_structure");
	connect(act, &QAction::triggered, _experiment, &Experiment::inflateMesh);
	_actions.push_back(act);
	act = structure->addAction(tr("Remove mesh"));
	act->setObjectName("need_structure");
	connect(act, &QAction::triggered, _experiment, &Experiment::removeMesh);
	_actions.push_back(act);

	QMenu *data = menuBar()->addMenu(tr("&Data"));
	_menus.push_back(data);
	act = data->addAction(tr("Load competition &data"));
	act->setObjectName("need_structure");
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::loadCSV);
	act = data->addAction(tr("Load binder &positions"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::loadPositions);
	_actions.push_back(act);
	act = data->addAction(tr("Load metadata"));
	act->setObjectName("need_data");
	connect(act, &QAction::triggered, this, &SurfaceView::loadMetadata);
	act = data->addAction(tr("Launch cluster&4x"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::launchCluster4x);

	_binders = menuBar()->addMenu(tr("&Antibodies"));
	_menus.push_back(_binders);
	
	_experiment->addBindersToMenu(_binders);
	
	act = _binders->addAction(tr("Make antibody..."));
	act->setObjectName("");
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::makeAntibody);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Write out antibody positions"));
	act->setObjectName("need_data");
	connect(act, &QAction::triggered, this, &SurfaceView::writeOutPositions);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Export PDB"));
	act->setObjectName("need_data");
	connect(act, &QAction::triggered, this, &SurfaceView::exportPDB);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Sequence alignment"));
	act->setObjectName("need_data");
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::alignSequences);
	_actions.push_back(act);
	
	QMenu *rec = _binders->addMenu(tr("Recolour by..."));
	rec->setObjectName("metadata");

	act = rec->addAction(tr("Recolour by correlation"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::recolourByCorrelation);

	if (_metadata->titleCount() > 0)
	{
		_metadata->makeMenu(rec, _experiment);
	}
	
	/*
	act = _binders->addAction(tr("Identify non-competitors"));
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::identifyNonCompetitors);
	_actions.push_back(act);
	
	act = _binders->addAction(tr("Distance-competition values"));
	connect(act, &QAction::triggered, 
	        this, &SurfaceView::plotDistanceCompetition);
	_actions.push_back(act);
	*/

	_binders->addSeparator();

	QMenu *refine = menuBar()->addMenu(tr("&Refine"));
	_menus.push_back(refine);
	act = refine->addAction(tr("&Refine this a bit more"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, this, &SurfaceView::bitMoreRefine);

	QMenu *mc = refine->addMenu(tr("&Monte Carlo"));
	_menus.push_back(mc);

	act = mc->addAction(tr("Open results"));
	connect(act, &QAction::triggered, _experiment, 
	        &Experiment::openResults);
	_actions.push_back(act);
	act = mc->addAction(tr("Run &1000 cycles"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::monteCarlo);
	act = mc->addAction(tr("Start"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::mCarloStart);
	act = mc->addAction(tr("Stop"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::mCarloStop);

	act = refine->addAction(tr("Randomise positions"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::randomise);
	act = refine->addAction(tr("Jiggle"));
	act->setObjectName("need_data");
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, &Experiment::jiggle);
	refine->addSeparator();

	act = refine->addAction(tr("Use correlation"));
	act->setObjectName("correlation");
	_actions.push_back(act);
	act->setCheckable(true);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->chooseTarget(TargetCorrelation); });

	act = refine->addAction(tr("Use least squares"));
	act->setObjectName("least_squares");
	_actions.push_back(act);
	act->setCheckable(true);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->chooseTarget(TargetLeastSquares); });

	act = refine->addAction(tr("Use both"));
	act->setObjectName("both");
	_actions.push_back(act);
	act->setCheckable(true);
	connect(act, &QAction::triggered, _experiment, 
	[=]() { _experiment->chooseTarget(TargetBoth); });
	refine->addSeparator();

	act = refine->addAction(tr("Relocate flying antibodies"));
	act->setObjectName("relocate");
	_actions.push_back(act);
	act->setCheckable(true);

	/*
	act = refine->addAction(tr("Enable elbow angles"));
	_actions.push_back(act);
	connect(act, &QAction::triggered, _experiment, 
	[=]() {_experiment->enableElbows();});
	*/
	
	updateMenu();
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

void SurfaceView::loadMetadata()
{
	std::string filename = openDialogue(this, "Choose metadata CSV", 
	                                    "Comma-separated values (*.csv)",
	                                    false);

	if (!checkFileIsValid(filename, false))
	{
		return;
	}

	_metadata->setFilename(filename);
	_metadata->loadBounds(_experiment->bounds());
	_metadata->load();
	
	makeMenu();
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
		_experiment->toggleLabels();
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

void SurfaceView::wheelEvent(QWheelEvent *e)
{
	double wheelY = e->angleDelta().y();
	double wheelX = e->angleDelta().x();

	if(abs(wheelX) > 0)
	{
		/* This will only be usefull for laptop users, or someone with
		2d mousewheel, but enables the ability to rotate the antigen
		only with the mousewheel */
		_gl->draggedLeftMouse(wheelX,wheelY);
	}
	else if (abs(wheelY) > 0)
	{
		/* minus sign fixes mouse wheel direction to
		align with expectation. */
		_gl->draggedRightMouse(0,-10*wheelY);
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
	if (!_moving)
	{
		// this was just a click, try to select
		double x = e->x(); double y = e->y();
		convertCoords(&x, &y);
		_experiment->clickMouse(x, y, _shiftPressed);
	}
	
	if (_moving && e->button() == Qt::LeftButton && _shiftPressed)
	{
		_experiment->finishDragging();
	}

	if (!_moving && e->button() == Qt::RightButton
	    && _experiment->getSelected() != NULL)
	{
		makeRightClickMenu(e->globalPos());
	}

	_mouseButton = Qt::NoButton;
}

void SurfaceView::writeOutPositions()
{
	std::string filename = openDialogue(this, "Choose position CSV", 
	                                    "Comma-separated values (*.csv)",
	                                    true);

	if (!checkFileIsValid(filename, true))
	{
		return;
	}

	_experiment->writeOutCSV(filename);
	
	makeMenu();
}

void SurfaceView::loadPositions()
{
	std::string filename = openDialogue(this, "Choose position CSV", 
	                                    "Comma-separated values (*.csv)");

	if (!checkFileIsValid(filename, false))
	{
		return;
	}

	_experiment->loadPositions(filename);
	
	makeMenu();
}

void SurfaceView::loadCSV()
{
	std::string filename = openDialogue(this, "Choose competition data CSV", 
	                                    "Comma-separated values (*.csv)");

	if (!checkFileIsValid(filename, false))
	{
		return;
	}

	_experiment->loadCSV(filename);
}

void SurfaceView::loadSurface()
{
	std::string filename = openDialogue(this, "Load surface file", 
	                                    "Wavefront object file (*.obj)");

	if (!checkFileIsValid(filename, false))
	{
		return;
	}

	_experiment->loadStructure(filename);
}

void SurfaceView::loadCoords()
{
	std::string filename = openDialogue(this, "Load structure coordinates", 
	                                    "Protein data bank file (*.pdb)");

	if (!checkFileIsValid(filename, false))
	{
		return;
	}

	_experiment->loadStructureCoords(filename);
}

void SurfaceView::bitMoreRefine()
{
	_experiment->refineFurther();
}

void SurfaceView::launchCluster4x()
{
	if (_screen != NULL)
	{
		_screen->show();
		return;
	}

	_screen = new Screen(NULL);
	_screen->setWindowTitle("cluster4x - mabscape");
	ClusterList *list = _screen->getList();
	AveCSV *csv = new AveCSV(NULL, "");
	csv->setList(list);

	csv->startNewCSV("Data");
	if (_experiment->getData())
	{
		_experiment->updateCSV(csv, 1);
	}
	csv->startNewCSV("Model");
	if (_experiment->getData())
	{
		_experiment->updateCSV(csv, 0);
	}
	csv->startNewCSV("Errors");
	if (_experiment->getData())
	{
		_experiment->updateCSV(csv, 2);
	}

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

void SurfaceView::identifyNonCompetitors()
{
	std::string filename = openDialogue(this, "Choose comma-separated antibodies", 
	                                    "Comma-separated values (*.csv)");

	if (!checkFileIsValid(filename, false))
	{
		return;
	}

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

void SurfaceView::alignSequences()
{
	if (!_metadata->hasTitle("heavy") && !_metadata->hasTitle("light"))
	{
		QMessageBox msg;
		msg.setText("First, load a metadata CSV file from the Data menu, "\
		            "listing antibody ID in the first column, heavy chain "\
		            "sequence under title \"heavy\", light chain sequence "\
		            "under title \"light\", and then try again.");
		msg.exec();
		return;
	}

	if (!_metadata->hasTitle("heavy") || !_metadata->hasTitle("light"))
	{
		QMessageBox msg;
		msg.setText("Warning: only loaded antibody sequence for one "
		            "chain! Continuing anyway...");
		msg.exec();
	}

	_genes->loadSequences(_metadata, this);
}

void SurfaceView::highlightResidues()
{
	Dialogue *d = new Dialogue(NULL, "Highlight residues",
	                           "List residues of choice",
	                           "305, 330-349",
	                           "Highlight");
	d->setWindow(this);
	d->setTag(DialogueHighlight);
	d->show();
}

void SurfaceView::receiveDialogue(DialogueType type, std::string result)
{
	if (_experiment->isSomethingHappening())
	{
		return;
	}

	if (type == DialogueHighlight && _experiment->structure())
	{
		_experiment->structure()->highlightResidues(result);
	}

	if (type == DialogueMakeAntibody && _experiment)
	{
		_experiment->makeNamedBound(result);
	}
}

void SurfaceView::exportPDB()
{
	if (_experiment->isSomethingHappening())
	{
		return;
	}

	std::string filename = openDialogue(this, "Choose PDB file", 
	                                    "Protein data bank file (*.pdb)",
	                                    true);

	_experiment->writePDB(filename);
}

void SurfaceView::makeRightClickMenu(QPoint p)
{
	QMenu *menu = new QMenu;
	menu->addAction("Recolour others by competition", _experiment,  
	                [=]() {_experiment->recolourByBoundErrors(1, 0);});
	menu->addAction("Recolour others by predicted", _experiment,  
	                [=]() {_experiment->recolourByBoundErrors(0, 1);});
	menu->addAction("Recolour others by error", _experiment,  
	                [=]() {_experiment->recolourByBoundErrors(1, -1);});

	if (_experiment->loadedSequences())
	{
		menu->addAction("... by heavy chain similarity", _experiment,  
		                [=]() {_experiment->recolourBySequence(true);});
		menu->addAction("... by light chain similarity", _experiment,  
		                [=]() {_experiment->recolourBySequence(false);});
	}

	menu->exec(p);
}

void SurfaceView::makeAntibody()
{
	Dialogue *d = new Dialogue(NULL, "Make antibody",
	                           "Antibody name",
	                           "",
	                           "Create");
	d->setWindow(this);
	d->setTag(DialogueMakeAntibody);
	d->show();

}
