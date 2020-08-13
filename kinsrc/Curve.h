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

#ifndef __abmap__Curve__
#define __abmap__Curve__

#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>
#include <float.h>
#include <QPen>
#include <iostream>
#include <QTreeWidgetItem>
#include <QObject>

typedef enum
{
	CurveUnknown,
	CurveOnOff,
	CurveCompetition,
	CurveBuffer
} CurveType;

class CurveView;
class QThread;
class KModel;

class Curve : public QObject, public QTreeWidgetItem
{
Q_OBJECT
public:
	Curve(QTreeWidget *w);

	void setFilename(std::string fn)
	{
		_filename = fn;
	}
	
	void setNickname(std::string n)
	{
		_nickname = n;
	}
	
	std::string nickname()
	{
		return _nickname;
	}

	void loadData();
	
	void drawCurve(CurveView *view);
	void setType(CurveType type)
	{
		_type = type;
	}
	
	bool isRegionSet(int r)
	{
		return !(_minRegion[r] == FLT_MAX && _maxRegion[r] == FLT_MAX);
	}
	
	void setRegion(int r, double min, double max);
	void getRegion(int r, double *min, double *max);
	
	CurveType type()
	{
		return _type;
	}
	
	void calculate();
	void toOriginal();
	
	std::vector<double> xs()
	{
		return _xs;
	}
	
	std::vector<double> ys()
	{
		return _ys;
	}
	
	void copyYs(Curve *c);
	
	void copyRegions(Curve *c);
	
	double *regionPtr(int r, bool min)
	{
		if (min)
		{
			return &_minRegion[r];
		}
		
		return &_maxRegion[r];
	}

	bool hasModel()
	{
		return (_model != NULL);
	}
	
	void setBuffer(Curve *buffer)
	{
		_buffer = buffer;
	}
	
	void subtract(double shift);
	
	void setFirstAb(Curve *ab)
	{
		_firstAb = ab;
	}
	
	void setSecondAb(Curve *ab)
	{
		_secondAb = ab;
	}
	
	void setCurveView(CurveView *v)
	{
		_lastView = v;
	}
public slots:
	void handleRefine();
signals:
	void refineModel();
protected:
	virtual QPen getPen();

	std::vector<double> _xs;
	std::vector<double> _ys;
	std::vector<double> _orgys;

	std::mutex _mut;
	std::condition_variable _cv;
private:
	QThread *_worker;
	CurveView *_lastView;
	KModel *_model;
	std::string _filename;
	std::string _nickname;
	Curve *_buffer;
	Curve *_firstAb;
	Curve *_secondAb;

	CurveType _type;
	double _minRegion[3];
	double _maxRegion[3];
};

#endif
