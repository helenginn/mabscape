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

#include "CurveView.h"
#include "Curve.h"
#include "KModel.h"
#include <iostream>
#include <cmath>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QPen>
#include <QTimer>

CurveView::CurveView(QWidget *parent) : QGraphicsView(parent)
{
	_moveable = true;
	_scene = new QGraphicsScene(this);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setScene(_scene);

	QBrush brush(Qt::transparent);
	setBackgroundBrush(brush);

	_mode = ModeNormal;

	_xmin = -100; _ymin = -0.1;
	_xmax = 1800; _ymax = 2.2;

	_xclick = -1;
	_yclick = -1;
	_regionStart = -1;
	_regionEnd = -1;

	_timer = new QTimer();
	_timer->setInterval(1000);
	_timer->setSingleShot(false);
	connect(_timer, &QTimer::timeout, this, &CurveView::redraw);

	show();
	setFocusPolicy(Qt::StrongFocus);
}

void CurveView::resizeEvent(QResizeEvent *)
{
	_scene->setSceneRect(0, 0, width(), height());
	redraw();
}

void CurveView::drawRegion()
{
	if (_mode == ModeNormal)
	{
		return;
	}

	double yrs = _regionStart;
	double yre = _regionEnd;
	QGraphicsRectItem *r;
	r = _scene->addRect(yrs, 0, yre - yrs, height());
	r->setBrush(Qt::yellow);
}

void CurveView::redraw()
{
	clear();
	
	drawRegion();
	drawAxes();
	
	QList<QTreeWidgetItem *> list = _tree->selectedItems();
	
	for (int i = 0; i < list.size(); i++)
	{
		Curve *c = static_cast<Curve *>(list[i]);
		c->drawCurve(this);
	}
}

void CurveView::clear()
{
	qDeleteAll(_scene->items());
	_scene->clear();
}

void CurveView::getRegion(double *x, double *y)
{
	*x = _regionStart;
	*y = _regionEnd;
	convertToGraphCoords(x, NULL);
	convertToGraphCoords(y, NULL);
}

void CurveView::convertToGraphCoords(double *x, double *y)
{
	if (x != NULL)
	{
		*x /= width();
		 *x *= (_xmax - _xmin);
		*x += _xmin;
	}

	if (y != NULL)
	{
		 *y = height() - *y;
		 *y /= height();
		 *y *= (_ymax - _ymin);
		*y += _ymin;
	}
}

void CurveView::convertCoords(double *x, double *y)
{
	if (x != NULL)
	{
		*x -= _xmin;
		 *x /= (_xmax - _xmin);
		 *x *= width();
	}

	if (y != NULL)
	{
		*y -= _ymin;
		 *y /= (_ymax - _ymin);
		 *y *= height();
		 *y = height() - *y;
	}
}

void CurveView::labelAxes()
{
	double base = 10;
	double spanx = _xmax - _xmin;
	double spany = _ymax - _ymin;

	double lgx = log(spanx) / log(base);
	double lgy = log(spany) / log(base);

	double stepx = pow(base, (int)ceil(lgx)) / base;
	double stepy = pow(base, (int)ceil(lgy)) / base;

	if (stepx / spanx < 0.15)
	{
		stepx *= 2;
	}
	else if (stepx / spanx > 0.2)
	{
		stepx /= 2;
	}

	if (stepy / spany < 0.15)
	{
		stepy *= 2;
	}
	else if (stepy / spany > 0.2)
	{
		stepy /= 2;
	}

	double startx = floor(_xmin / stepx) * stepx;
	
	for (double i = startx; ; i += stepx)
	{
		if (fabs(i) < 1e-6)
		{
			continue;
		}

		double val = i;
		double zero = 0;
		convertCoords(&val, &zero);
		
		if (val / (double)width() > 1)
		{
			break;
		}

		QGraphicsTextItem *label;
		label = _scene->addText(QString::number(i));
		label->setPos(val, zero + 5);
	}

	double starty = floor(_ymin / stepy) * stepy;
	
	for (double i = starty; ; i += stepy)
	{
		if (fabs(i) < 1e-6)
		{
			continue;
		}

		double val = i;
		double zero = 0;
		convertCoords(&zero, &val);
		
		if (val / (double)width() < 0)
		{
			break;
		}

		QGraphicsTextItem *label;
		label = _scene->addText(QString::number(i));
		label->setPos(zero - label->textWidth(), val);
	}
}

void CurveView::drawAxes()
{
	QPen p = QPen(QColor(0, 0, 0));
	double zero_x = 0;
	double zero_y = 0;
	
	convertCoords(&zero_x, &zero_y);

	_scene->addLine(zero_x, 0, zero_x, height(), p);
	_scene->addLine(0, zero_y, width(), zero_y, p);

	QGraphicsTextItem *origin = _scene->addText("0");
	origin->setPos(zero_x - 20, 
	               zero_y + 5);
	
	labelAxes();
}

void CurveView::mousePressEvent(QMouseEvent *e)
{
	_xclick = e->x();
	_yclick = e->y();
	_button = e->button();
	
	if (_mode == ModeSetRegion)
	{
		_regionStart = _xclick;
	}
}

void CurveView::mouseReleaseEvent(QMouseEvent *e)
{
	_xclick = -1;
	_yclick = -1;
	
	if (_mode == ModeSetRegion)
	{
		_mode = ModeNormal;
		emit finishedRegionSet();
	}
}

void CurveView::zoom(QMouseEvent *e)
{
	double my = _yclick - e->y();
	my /= (double)height();
	if (my > 0.1) my = 0.1;
	if (my < -0.1) my = -0.1;
	my++;

	double _midx = (_xmax + _xmin) / 2;
	double _midy = (_ymax + _ymin) / 2;
	double sy = (_ymax - _ymin) * my;
	double sx = (_xmax - _xmin) * my;

	_xmin = _midx - sx / 2;
	_ymin = _midy - sy / 2;
	_xmax = _midx + sx / 2;
	_ymax = _midy + sy / 2;

	redraw();
}

void CurveView::dragRegion(QMouseEvent *e)
{
	_regionEnd = e->x();

	redraw();

}

void CurveView::drag(QMouseEvent *e)
{
	double mx = e->x() - _xclick;
	double my = _yclick - e->y();
	mx /= (double)width();
	my /= (double)height();

	mx *= (_xmax - _xmin);
	my *= (_ymax - _ymin);
	
	_xmax -= mx; _xmin -= mx;
	_ymax -= my; _ymin -= my;

	redraw();

}

void CurveView::mouseMoveEvent(QMouseEvent *e)
{
	if (_xclick < 0 || _yclick < 0 || !_moveable)
	{
		return;
	}
	
	if (_button == Qt::LeftButton && _mode == ModeNormal)
	{
		drag(e);
	}
	else if (_button == Qt::LeftButton && _mode == ModeSetRegion)
	{
		dragRegion(e);
	}
	else if (_button == Qt::RightButton && _mode == ModeNormal)
	{
		zoom(e);
	}
	
	_xclick = e->x();
	_yclick = e->y();
}

void CurveView::hookModel(KModel *model)
{
	_timer->start();
}

void CurveView::startTimer()
{
	_timer->start();
}

void CurveView::unhookModel(KModel *model)
{
	_timer->stop();
	redraw();
}
