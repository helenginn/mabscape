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

#ifndef __abmap__CurveView__
#define __abmap__CurveView__

#include <QGraphicsView>
#include <vector>

class Curve;
class KModel;
class QMouseEvent;
class QTreeWidget;

typedef enum
{
	ModeNormal,
	ModeSetRegion,
} GraphMode;

class CurveView : public QGraphicsView
{
Q_OBJECT
public:
	CurveView(QWidget *parent = NULL);
	
	QGraphicsScene *scene()
	{
		return _scene;
	}

	size_t curveCount()
	{
		return _curves.size();
	}
	
	Curve *curve(int i)
	{
		return _curves[i];
	}
	
	void convertCoords(double *x, double *y);
	void convertToGraphCoords(double *x, double *y);
	
	void setCurveTree(QTreeWidget *tree)
	{
		_tree = tree;
	}
	
	void setMode(GraphMode mode)
	{
		_mode = mode;
	}
	
	GraphMode mode()
	{
		return _mode;
	}
	
	void startTimer();

	void getRegion(double *x, double *y);
	void hookModel(KModel *model);
	void unhookModel(KModel *model);
public slots:
	void redraw();
signals:
	void finishedRegionSet();

protected:
	virtual void resizeEvent(QResizeEvent *);
	
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
private:
	void drag(QMouseEvent *e);
	void dragRegion(QMouseEvent *e);
	void zoom(QMouseEvent *e);
	void clear();
	void drawRegion();
	void drawAxes();
	void labelAxes();

	QGraphicsScene *_scene;
	QTimer *_timer;
	
	double _xmin, _xmax;
	double _ymin, _ymax;
	
	bool _moveable;
	int _regionStart;
	int _regionEnd;
	int _xclick, _yclick;
	GraphMode _mode;
	std::vector<Curve *> _curves;
	Qt::MouseButton _button;
	QTreeWidget *_tree;
};

#endif
