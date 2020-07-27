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

#include <string>
#include <vector>

class SurfaceView;
class SlipGL;
class Structure;
class Bound;
class QLabel;

class Experiment
{
public:
	Experiment(SurfaceView *view);
	
	void setGL(SlipGL *gl)
	{
		_gl = gl;
	}

	void loadStructure(std::string filename);
	void loadBound(std::string filename);
	void hoverMouse(double x, double y);
	void clickMouse(double x, double y);
	void checkDrag(double x, double y);
	void finishDragging();
	void drag(double x, double y);
	void hideLabel();
	void fixBound();
	void loadCSV(std::string filename);
private:
	Bound *findBound(double x, double y);
	void select(Bound *bound, double x, double y);
	void convertCoords(double *x, double *y);
	void deselectAll();
	void dehighlightAll();
	bool _dragging;
	SurfaceView *_view;
	SlipGL *_gl;

	Structure *_structure;
	std::vector<Bound *> _bounds;
	Bound *_selected;
	QLabel *_label;
};

#endif
