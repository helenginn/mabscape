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

#ifndef __abmap__Experiment__
#define __abmap__Experiment__

#include "Refinement.h"
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <QObject>
#include "PositionMap.h"

class Metadata;
class SurfaceView;
class Explorer;
class SlipGL;
class Structure;
class Bound;
class Refinement;
class Data;
class AveCSV;
class QMenu;
class Result;
class Mesh;
class QLabel;
class QThread;
class PatchView;

class Experiment : public QObject
{
Q_OBJECT
public:
	Experiment(SurfaceView *view);
	
	SurfaceView *getView()
	{
		return _view;
	}
	
	void setGL(SlipGL *gl)
	{
		_gl = gl;
	}
	
	Refinement *refinement()
	{
		return _refinement;
	}
	
	void setBoundObj(std::string filename)
	{
		_boundObj = filename;
	}
	
	Data *getData()
	{
		return _data;
	}
	
	AveCSV *csv();
	void updateCSV(AveCSV *csv, int data);
	
	size_t boundCount()
	{
		return _bounds.size();
	}
	
	Bound *bound(int i)
	{
		return _bounds[i];
	}

	Bound *bound(std::string name)
	{
		return _nameMap[name];
	}
	
	std::vector<Bound *> bounds()
	{
		return _bounds;
	}
	
	Bound *getSelected()
	{
		return _selected;
	}
	
	Structure *structure()
	{
		return _structure;
	}
	
	Explorer *getExplorer()
	{
		return _explorer;
	}
	
	Metadata *metadata()
	{
		return _metadata;
	}
	
	void setMetadata(Metadata *md)
	{
		_metadata = md;
	}

	void makeExplorer();
	void relocateFliers(bool relocate);
	void loadStructure(std::string filename);
	void loadStructureCoords(std::string filename);
	void loadPositions(std::string filename);
	void hoverMouse(double x, double y, bool shift);
	void clickMouse(double x, double y);
	void checkDrag(double x, double y);
	void finishDragging();
	void drag(double x, double y);
	void hideLabel();
	void fixBound();
	void fixLabel();
	void addBindersToMenu(QMenu *menu);
	bool prepareWorkForMesh();
	bool prepareWorkForObject(QObject *object);
	void loadCSV(std::string filename);
	void somethingToCluster4x(int data);
	void refineFurther();
	void refineModel(bool fixedOnly, bool svd = false);
	void findNonCompetitors(std::vector<std::string> abs);
	void plotDistanceCompetition();
	
	void setPassToResults(bool pass)
	{
		_passToResults = pass;
	}
	
	void setMonteTarget(int target)
	{
		_monteTarget = target;
	}
	
	QThread *worker();
signals:
	void refine();
public slots:
	void addBindersToMenu();
	void handleMesh();
	void abPatchwork(std::string id);
	void jiggle();
	void randomise();
	void openResults();
	void fixFromPDB();
	void recolourByBoundErrors();
	void recolourByCorrelation();
	void selectFromMenu();
	void handleResults();
	void handleError();
	void oneTimeResults();
	QThread *monteCarlo();
	void mCarloStart();
	void mCarloStop();
	void refineMesh();
	void removeMesh();
	void smoothMesh();
	void inflateMesh();
	void triangulateMesh();
	void triangulateStructure();
	void meshStructure();
	QThread *runSmoothMesh();
	QThread *runInflateMesh();
	void writeOutCSV(std::string filename);
	void chooseTarget(Target t);
private:
	void drawWindow(double x, double y);
	bool isRunningMonteCarlo();
	void updateWindow();
	void addNonCompetitor(std::vector<std::string> abs,
	                      std::vector<std::string> &chain,
	                      std::vector<std::string> *addable);
	void createBinders();
	Bound *findBound(double x, double y);
	Bound *loadBound(std::string filename);
	void select(Bound *bound, double x, double y);
	void convertCoords(double *x, double *y);
	void monteCarloRound();
	void finishUpMonteCarlo();
	void deselectAll();
	void dehighlightAll();
	bool _dragging;

	SurfaceView *_view;
	PatchView *_patchView;
	SlipGL *_gl;
	Data *_data;
	Refinement *_refinement;

	std::map<std::string, Bound *> _nameMap;
	Structure *_structure;
	std::vector<Bound *> _bounds;
	Bound *_selected;
	QLabel *_label;
	QThread *_worker;
	std::mutex _mut;

	Metadata *_metadata;
	Mesh *_mesh;
	int _monteCount;
	int _monteTarget;
	Explorer *_explorer;
	std::vector<Result *> _results;
	
	std::string _boundObj;
	bool _passToResults;

	QLabel *_selWindow;
	bool _windowing;
	double _winsx, _winsy, _winex, _winey;
};

#endif
