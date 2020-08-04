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

#ifndef __abmap__Explorer__
#define __abmap__Explorer__

#include <QMainWindow>
#include <QTreeWidget>
#include "SlipObject.h"

class Result;
class Experiment;
class Squiggle;
class Bound;

class Explorer : public QMainWindow, public SlipObject
{
Q_OBJECT
public:
	Explorer(QWidget *parent);

	void addResults(std::vector<Result *> results);
	
	void setExperiment(Experiment *exp)
	{
		_experiment = exp;
	}

	void highlightBound(Bound *bi);
	
	virtual void render(SlipGL *gl);
	void clear();
private slots:
	void currentItemChanged(QTreeWidgetItem *current, 
	                        QTreeWidgetItem *prev);
	void itemSelectionChanged();
private:
	Squiggle *_squiggle;

	Experiment *_experiment;
	QTreeWidget *_widget;
};

#endif
