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
#include "PatchView.h"
#include "Patch.h"
#include "Experiment.h"
#include "SlipGL.h"
#include <QThread>
#include <iostream>

PatchView::PatchView() : QMainWindow(NULL)
{
	_lastX = -1;
	_lastY = -1;
	_gl = new SlipGL(this);
	_gl->show();
	_gl->updateProjection(2.0);
	_controlPressed = false;
	_moving = false;
	_shiftPressed = false;
	resize(1000, 1000);
	std::cout << QThread::currentThread() << std::endl;
}

void PatchView::project()
{
	Structure *s = _experiment->structure();
	Patch *p = new Patch(s);
	p->setPatchView(this);
	p->setTitle(_title);
	p->setCentre(_centre);
	p->project();
	_gl->addObject(p, true);
	_patch = p;
}

void PatchView::resizeEvent(QResizeEvent *event)
{
	_gl->setGeometry(0, 0, width(), height());
}


void PatchView::mousePressEvent(QMouseEvent *e)
{
	_lastX = e->x();
	_lastY = e->y();
	_mouseButton = e->button();
	_moving = false;
}


void PatchView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Alt || event->key() == Qt::Key_Control)
	{
		_controlPressed = true;
	}
	else if (event->key() == Qt::Key_Shift)
	{
		_shiftPressed = true;
	}
}

void PatchView::keyReleaseEvent(QKeyEvent *event)
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

void PatchView::mouseMoveEvent(QMouseEvent *e)
{
	double newX = e->x();
	double xDiff = _lastX - newX;
	double newY = e->y();
	double yDiff = _lastY - newY;
	_lastX = newX;
	_lastY = newY;

	if (_moving && _mouseButton == Qt::LeftButton)
	{
		if (_controlPressed)
		{
			_patch->panned(xDiff / PAN_SENSITIVITY / PAN_SENSITIVITY, 
			               -yDiff / PAN_SENSITIVITY / PAN_SENSITIVITY);
		}
		else
		{
			_patch->rotated(-xDiff / PAN_SENSITIVITY, 
			                -yDiff / PAN_SENSITIVITY);
		}
	}
	else if (_moving && _mouseButton == Qt::RightButton)
	{
		_patch->zoom(yDiff / PAN_SENSITIVITY / PAN_SENSITIVITY);
	}

	_moving = true;
}

void PatchView::mouseReleaseEvent(QMouseEvent *e)
{
	_lastX = -1;
	_lastY = -1;

	if (!_moving && e->button() == Qt::LeftButton)
	{
		// this was just a click
		// nothing
	}
	
	if (_moving && e->button() == Qt::LeftButton && _shiftPressed)
	{
		_experiment->finishDragging();
	}
	
	_mouseButton = Qt::NoButton;
}
