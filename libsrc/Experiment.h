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
#include <map>
#include <QObject>
#include "PositionMap.h"

class SurfaceView;
class Explorer;
class SlipGL;
class Structure;
class Bound;
class Refinement;
class Data;
class QMenu;
class Result;
class Mesh;
class QLabel;
class QThread;

class Experiment : public QObject
{
Q_OBJECT
public:
	Experiment(SurfaceView *view);
	
	void setGL(SlipGL *gl)
	{
		_gl = gl;
	}
	
	void setBoundObj(std::string filename)
	{
		_boundObj = filename;
	}
	
	Data *getData()
	{
		return _data;
	}
	
	size_t boundCount()
	{
		return _bounds.size();
	}
	
	Bound *bound(int i)
	{
		return _bounds[i];
	}
	
	Structure *structure()
	{
		return _structure;
	}

	void loadStructure(std::string filename);
	void loadPositions(std::string filename);
	void hoverMouse(double x, double y);
	void clickMouse(double x, double y);
	void checkDrag(double x, double y);
	void finishDragging();
	void drag(double x, double y);
	void hideLabel();
	void fixBound();
	void addBindersToMenu(QMenu *menu);
	void loadCSV(std::string filename);
	void refineModel(bool fixedOnly, bool svd = false);
	
	void setPassToResults(bool pass)
	{
		_passToResults = pass;
	}
signals:
	void refine();
public slots:
	void handleMesh();
	void jiggle();
	void randomise();
	void svdRefine();
	void clearMonteCarlo();
	void recolourByCorrelation();
	void selectFromMenu();
	void handleResults();
	void handleError();
	void monteCarlo();
	void refineMesh();
	void removeMesh();
	void smoothMesh();
	void inflateMesh();
	void triangulateMesh();
	void meshStructure();
	void writeOutCSV();
	void chooseTarget(Target t);
private:
	bool prepareWorkForMesh();
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
	SlipGL *_gl;
	Data *_data;
	Refinement *_refinement;

	std::map<std::string, Bound *> _nameMap;
	Structure *_structure;
	std::vector<Bound *> _bounds;
	Bound *_selected;
	QLabel *_label;
	QThread *_worker;

	Mesh *_mesh;
	int _monteCount;
	Explorer *_explorer;
	std::vector<Result *> _results;
	
	std::string _boundObj;
	bool _passToResults;
};

#endif
