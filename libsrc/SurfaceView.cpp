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
#include "SurfaceView.h"
#include "Structure.h"

SurfaceView::SurfaceView(QWidget *p) : QMainWindow(p)
{
	_gl = new SlipGL(this);
	_gl->show();

	resize(1000, 1000);
	setFocus();
}

void SurfaceView::resizeEvent(QResizeEvent *event)
{
	_gl->setGeometry(0, 0, width(), height());
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
	if (_mouseButton == Qt::NoButton)
	{
		return;
	}

	_moving = true;
	
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
			_gl->panned(xDiff * 2, yDiff * 2);
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

void SurfaceView::convertCoords(double *x, double *y)
{
	double w = width();
	double h = height();

	*x = 2 * *x / w - 1.0;
	*y =  - (2 * *y / h - 1.0);
}

void SurfaceView::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_moving)
	{
		// this was just a click
	}
	
	_mouseButton = Qt::NoButton;
}

void SurfaceView::loadStructure(std::string filename)
{
	Structure *str = new Structure(filename);
	_gl->addObject(str, true);

}
