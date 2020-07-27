// Spikey thing
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

#ifndef __abmap__SurfaceView__
#define __abmap__SurfaceView__

#include <QMainWindow>
#include <QMouseEvent>

class Bound;
class SlipGL;
class QTimer;
class QMenu;
class QAction;
class Experiment;

class SurfaceView : public QMainWindow
{
Q_OBJECT
public:
	SurfaceView(QWidget *p = NULL);
	
	Experiment *getExperiment()
	{
		return _experiment;
	}

	~SurfaceView()
	{

	}
	
	void addMenu(QMenu *men)
	{
		_menus.push_back(men);
	}
	
	void addAction(QAction *act)
	{
		_actions.push_back(act);
	}

	void convertToViewCoords(double *x, double *y);
protected:
	virtual void resizeEvent(QResizeEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
private slots:
	void loadCSV();
	void unrestrainedRefine();
	void fixToSurfaceRefine();
private:
	void makeMenu();
	void convertCoords(double *x, double *y);
	Experiment *_experiment;
	SlipGL *_gl;
	Qt::MouseButton _mouseButton;
	bool _controlPressed;
	bool _shiftPressed;
	double _lastX; double _lastY;
	bool _moving;

	std::vector<QMenu *> _menus;
	std::vector<QAction *> _actions;
};

#endif
