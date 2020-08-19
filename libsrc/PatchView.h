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

#ifndef __abmap__PatchView__
#define __abmap__PatchView__

#include <QMouseEvent>
#include <QMainWindow>
#include <vec3.h>

class SlipGL;
class Patch;
class Experiment;

class PatchView : public QMainWindow
{
Q_OBJECT
public:
	PatchView();

	void setExperiment(Experiment *exp)
	{
		_experiment = exp;
	}

	void setCentre(vec3 v)
	{
		_centre = v;
	}
	
	void setTitle(std::string title)
	{
		_title = title;
	}
	
public slots:
	void project();

protected:
	virtual void resizeEvent(QResizeEvent *event);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);
private:
	Experiment *_experiment;
	SlipGL *_gl;
	Patch *_patch;
	Qt::MouseButton _mouseButton;
	bool _controlPressed;
	bool _shiftPressed;
	double _lastX; double _lastY;
	bool _moving;

	vec3 _centre;
	std::string _title;
};

#endif
