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

#include "Experiment.h"
#include "SurfaceView.h"
#include "SlipGL.h"
#include "Bound.h"
#include "Structure.h"
#include "FileReader.h"
#include <iomanip>
#include <float.h>
#include <QLabel>

Experiment::Experiment(SurfaceView *view)
{
	_selected = NULL;
	_dragging = false;
	_view = view;
	_gl = NULL;
	_structure = NULL;
	_label = new QLabel("", _view);
	_label->setAlignment(Qt::AlignVCenter);
	_label->setAlignment(Qt::AlignHCenter);
	QFont font = QFont("Helvetica", 16);
	_label->setFont(font);
}


void Experiment::loadStructure(std::string filename)
{
	Structure *str = new Structure(filename);
	_gl->addObject(str, true);
	_structure = str;
}


void Experiment::loadBound(std::string filename)
{
	Bound *bnd = new Bound(filename);
	bnd->setName("thing" + i_to_str(_bounds.size() + 1));
	_gl->addObject(bnd, false);
	
	if (_structure)
	{
		bnd->randomlyPositionInRegion(_structure);
		bnd->snapToObject(_structure);
	}

	_bounds.push_back(bnd);
}

Bound *Experiment::findBound(double x, double y)
{
	Bound *which = NULL;
	double z = -FLT_MAX;

	for (size_t i = 0; i < _bounds.size(); i++)
	{
		bool covers = _bounds[i]->intersects(x, y, &z);
		
		if (covers)
		{
			which = _bounds[i];
		}
	}

	return which;
}

void Experiment::hoverMouse(double x, double y)
{
	Bound *which = findBound(x, y);

	if (which != NULL)
	{
		dehighlightAll();
		which->setHighlighted(true);
	}
	else
	{
		dehighlightAll();
	}
}

void Experiment::dehighlightAll()
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		_bounds[i]->setHighlighted(false);
	}
}

void Experiment::deselectAll()
{
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		_bounds[i]->setSelected(false);
	}
	
	_selected = NULL;
	hideLabel();
}

void Experiment::select(Bound *bound, double x, double y)
{
	_selected = bound;
	bound->setSelected(true);
	QString name = QString::fromStdString(bound->name());
	_view->convertToViewCoords(&x, &y);
	_label->setGeometry(x - 50, y - 50, 100, 50);
	_label->setText(name);
	_label->show();
}

void Experiment::checkDrag(double x, double y)
{
	if (_selected == NULL || _selected->isFixed())
	{
		_dragging = false;
		return;
	}

	double z = -FLT_MAX;

	_dragging = _selected->intersects(x, y, &z);
}

void Experiment::drag(double x, double y)
{
	if (_selected == NULL || !_dragging)
	{
		return;
	}

	_selected->changeMidPoint(x, y);
}

void Experiment::clickMouse(double x, double y)
{
	Bound *which = findBound(x, y);

	if (which != NULL)
	{
		std::cout << "Selected: " << which->name() << std::endl;
		deselectAll();
		select(which, x, y);
	}
	else
	{
		deselectAll();
	}
}

void Experiment::hideLabel()
{
	_label->setText("");
	_label->hide();
}

void Experiment::finishDragging()
{
	_dragging = false;
	if (_selected)
	{
		_selected->snapToObject(_structure);
	}
}

void Experiment::fixBound()
{
	if (_selected)
	{
		_selected->toggleFixPosition();
	}
}
