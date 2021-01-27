// mabscape
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

#include "ColourOptions.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>

ColourOptions::ColourOptions(QWidget *parent) : QMainWindow(parent)
{
	QWidget *window = new QWidget();
	QVBoxLayout *box = new QVBoxLayout();
	window->setLayout(box);
	
	QButtonGroup *group = new QButtonGroup(this);

	{
		QRadioButton *r = new QRadioButton("By scale", window);
		group->addButton(r);
		box->addWidget(r);
	}

	{
		QButtonGroup *gr = new QButtonGroup(this);
		QHBoxLayout *hbox = new QHBoxLayout();
		hbox->addSpacing(10);
		QRadioButton *r1 = new QRadioButton("Using mean", window);
		hbox->addWidget(r1);
		gr->addButton(r1);
		QRadioButton *r2 = new QRadioButton("or set mean:", window);
		hbox->addWidget(r2);
		gr->addButton(r2);
		QLineEdit *e = new QLineEdit(window);
		hbox->addWidget(e);
		box->addLayout(hbox);
	}

	{
		QButtonGroup *gr = new QButtonGroup(this);
		QHBoxLayout *hbox = new QHBoxLayout();
		hbox->addSpacing(10);
		QRadioButton *r1 = new QRadioButton("Using stdev", window);
		hbox->addWidget(r1);
		gr->addButton(r1);
		QRadioButton *r2 = new QRadioButton("or set stdev:", window);
		hbox->addWidget(r2);
		gr->addButton(r2);
		QLineEdit *e = new QLineEdit(window);
		hbox->addWidget(e);
		box->addLayout(hbox);
	}

	{
		QRadioButton *r = new QRadioButton("By unique group", window);
		group->addButton(r);
		box->addWidget(r);
	}

	{
		QPushButton *b = new QPushButton("Colour", window);
		box->addWidget(b);
	}

	setCentralWidget(window);
}
