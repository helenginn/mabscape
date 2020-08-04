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

#include "Explorer.h"
#include "Structure.h"
#include "Mesh.h"
#include "Experiment.h"
#include "Result.h"
#include "Bound.h"
#include "Squiggle.h"

Explorer::Explorer(QWidget *parent) : QMainWindow(parent)
{
	setGeometry(200, 200, 500, 500);
	setWindowTitle("Results");

	_widget = new QTreeWidget(this);
	_widget->setGeometry(0, 0, 200, 500);
	_widget->show();
	
	connect(_widget, &QTreeWidget::currentItemChanged,
	        this, &Explorer::currentItemChanged);
	connect(_widget, &QTreeWidget::itemSelectionChanged,
	        this, &Explorer::itemSelectionChanged);
	
	_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	
	_squiggle = new Squiggle();
	_experiment = NULL;
	initializeOpenGLFunctions();
}

void Explorer::clear()
{
	_widget->clear();
}

void Explorer::addResults(std::vector<Result *> results)
{
	for (size_t i = 0; i < results.size(); i++)
	{
		_widget->addTopLevelItem(results[i]);
	}


	_widget->sortItems(0, Qt::AscendingOrder);
}

void Explorer::currentItemChanged(QTreeWidgetItem *current, 
                                  QTreeWidgetItem *prev)
{
	if (current == NULL)
	{
		return;
	}

	Result *r = static_cast<Result *>(current);
	r->applyPositions(_experiment);
	
}

void Explorer::itemSelectionChanged()
{
	double alpha = 1.;
	
	if (_widget->selectedItems().size() > 1)
	{
		alpha = 0.5;
		_experiment->setPassToResults(true);
		
		if (_experiment->structure()->hasMesh())
		{
			_experiment->structure()->mesh()->setAlpha(0.1);
		}
	}
	else
	{
		_experiment->setPassToResults(false);
		_squiggle->clear();
		
		if (_experiment->structure()->hasMesh())
		{
			_experiment->structure()->mesh()->setAlpha(1.0);
		}
	}

	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		bi->setAlpha(alpha);
	}
}

void Explorer::highlightBound(Bound *bi)
{
	if (bi == NULL)
	{
		_squiggle->clear();
		return;
	}

	QList<QTreeWidgetItem *> list = _widget->selectedItems();
	
	std::vector<vec3> poz;

	for (int i = 0; i < list.size(); i++)
	{
		QTreeWidgetItem *item = list[i];
		Result *r = static_cast<Result *>(item);
		vec3 pos = r->vecForBound(bi);
		poz.push_back(pos);
	}
	
	_squiggle->setPositions(poz);
}

void Explorer::render(SlipGL *gl)
{
	glDisable(GL_DEPTH_TEST);
	_squiggle->render(gl);
	glEnable(GL_DEPTH_TEST);
}
