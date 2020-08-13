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

#include "Curve.h"
#include "KModel.h"
#include "CurveView.h"
#include <FileReader.h>
#include <iostream>
#include <QThread>
#include <QGraphicsView>
#include <QGraphicsLineItem>

#include "KExpDecay.h"
#include "KSecond.h"

Curve::Curve(QTreeWidget *w) : QTreeWidgetItem(w)
{
	_firstAb = NULL;
	_secondAb = NULL;
	_buffer = NULL;
	_model = NULL;
	_type = CurveUnknown;
	_worker = NULL;
	
	for (int i = 0; i < 3; i++)
	{
		_minRegion[i] = FLT_MAX;
		_maxRegion[i] = FLT_MAX;
	}
}

void Curve::loadData()
{
	std::string simple = getFilename(_filename);
	setText(0, simple.c_str());
	
	if (_nickname.length())
	{
		std::string title = _nickname + " (" + simple + ")";
		setText(0, title.c_str());
	}

	std::string contents = get_file_contents(_filename);
	std::vector<std::string> lines = split(contents, '\n');
	
	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> components = split(lines[i], ',');
		
		if (components.size() != 2)
		{
			continue;
		}

		char *pos1 = NULL;
		char *pos2 = NULL;
		double val1 = strtod(&components[0][0], &pos1);
		double val2 = strtod(&components[1][0], &pos2);

		if (pos2 == &(components[1][0]) || pos1 == &(components[0][0]))
		{
			if (i == 0)
			{
				std::cout << "First line of " << _filename << 
				" appears to be header." << std::endl;
			}
			else
			{
				std::cout << "Values of line " << i << " not numbers?"
				<< std::endl;
			}

			continue;
		}
		
		_xs.push_back(val1);
		_ys.push_back(val2);
	}
	
	_orgys = _ys;
}

void Curve::drawCurve(CurveView *view)
{
	_lastView = view;
	QGraphicsScene *_scene = view->scene();
	QPen pen = getPen();
	
	std::vector<double> tmpx, tmpy;
	
	/*
	if (!_mut.try_lock())
	{
		return;
	}
	*/
	_mut.lock();

	tmpx = _xs;
	tmpy = _ys;

	_mut.unlock();
	
	if (tmpx.size() < 2 || tmpy.size() < 2)
	{
		return;
	}

	double x = tmpx[0];
	double y = tmpy[0];
	view->convertCoords(&x, &y);

	for (size_t i = 0; i < tmpx.size(); i++)
	{
		double newx = tmpx[i];
		double newy = tmpy[i];
		view->convertCoords(&newx, &newy);
		
		QGraphicsLineItem *line = _scene->addLine(x, y, newx, newy, pen);
		line->setPen(pen);
		x = newx;
		y = newy;
	}

	if (_model != NULL)
	{
		_model->drawCurve(view);
	}
}

QPen Curve::getPen()
{
	return QPen(QColor(0, 0, 0));

}

void Curve::getRegion(int r, double *min, double *max)
{
	*min = _minRegion[r];
	*max = _maxRegion[r];

}

void Curve::setRegion(int r, double min, double max)
{
	_minRegion[r] = min;
	_maxRegion[r] = max;
}

void Curve::calculate()
{
	if (_worker && _worker->isRunning())
	{
		return;
	}
	
	if (_worker == NULL)
	{
		_worker = new QThread();
	}

	if (_model == NULL)
	{
		if (_type == CurveBuffer)
		{
			std::cout << "Buffer type" << std::endl;
			_model = new KExpDecay();
		}
		else if (_type == CurveOnOff)
		{
			std::cout << "On/off type" << std::endl;
			_model = new KLigOnOff();
		}
		else if (_type == CurveCompetition)
		{
			std::cout << "Curve competition" << std::endl;
			KSecond *comp = new KSecond();
			_model = comp;
			
			if (_firstAb == NULL || _secondAb == NULL)
			{
				std::cout << "Proper error message!" << std::endl;
				return;
			}
			
			KLigOnOff *onoff1 = dynamic_cast<KLigOnOff *>(_firstAb->_model);
			KLigOnOff *onoff2 = dynamic_cast<KLigOnOff *>(_secondAb->_model);
			comp->setFirstModel(onoff1);
			comp->setSecondModel(onoff2);
		}
		
		if (_type == CurveCompetition || _type == CurveOnOff)
		{
			if (_buffer == NULL)
			{
				std::cout << "Proper error message" << std::endl;
				return;
			}

			KExpDecay *decay = dynamic_cast<KExpDecay *>(_buffer->_model);
			
			if (decay != NULL)
			{
				KLigOnOff *kexp = static_cast<KLigOnOff *>(_model);
				kexp->setExponentialDecayModel(decay);
			}

		}

		if (!_model)
		{
			return;
		}
	}

	_model->setNickname(_nickname);
	_model->setCurve(this);

	QString me = QThread::currentThread()->objectName();

	if (me == "kintroller")
	{
		/* already background thread */
		_model->refineCascade();
	}
	else
	{
		_model->moveToThread(_worker);
		_lastView->hookModel(_model);
		connect(this, &Curve::refineModel, _model, &KModel::refineThenDone);
		connect(_model, &KModel::done, this, &Curve::handleRefine);
		_worker->start();
		emit refineModel();
	}
}

void Curve::handleRefine()
{
	_worker->quit();
	_worker->wait();

	std::cout << "Handling" << std::endl;
	_lastView->unhookModel(_model);
}

void Curve::copyYs(Curve *c)
{
	_ys = c->ys();
}

void Curve::copyRegions(Curve *c)
{
	for (int i = 0; i < 3; i++)
	{
		_minRegion[i] = c->_minRegion[i];
		_maxRegion[i] = c->_maxRegion[i];
	}
}

void Curve::toOriginal()
{
	if (_worker && _worker->isRunning())
	{
		std::cout << "Proper error message" << std::endl;
		return;
	}

	_ys = _orgys;
	if (_model != NULL)
	{
		delete _model;
		_model = NULL;
	}
}

void Curve::subtract(double shift)
{
	for (size_t i = 0; i < _ys.size(); i++)
	{
		_ys[i] += shift;
	}
}
