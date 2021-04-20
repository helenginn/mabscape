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
#include "Metadata.h"
#include <cmath>
#include <iostream>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>

ColourOptions::ColourOptions(QWidget *parent, Metadata *m) 
: QMainWindow(parent)
{
	QWidget *window = new QWidget();
	QVBoxLayout *box = new QVBoxLayout();
	window->setLayout(box);
	_m = m;
	
	QButtonGroup *group = new QButtonGroup(this);

	{
		QRadioButton *r = new QRadioButton("By scale", window);
		r->setObjectName("by_scale");
		r->setChecked(true);
		group->addButton(r);
		box->addWidget(r);
	}

	{
		QCheckBox *ch = new QCheckBox("Take log", this);
		ch->setObjectName("take_log");
		box->addWidget(ch);
	}

	{
		QButtonGroup *gr = new QButtonGroup(this);
		QHBoxLayout *hbox = new QHBoxLayout();
		hbox->addSpacing(10);
		QRadioButton *r1 = new QRadioButton("Using mean", window);
		r1->setObjectName("using_mean");
		r1->setChecked(true);
		hbox->addWidget(r1);
		gr->addButton(r1);
		QRadioButton *r2 = new QRadioButton("or set mean:", window);
		r2->setObjectName("set_mean");
		hbox->addWidget(r2);
		gr->addButton(r2);
		QLineEdit *e = new QLineEdit(window);
		e->setObjectName("write_mean");
		connect(e, &QLineEdit::textChanged,
		        r2, [=]() {r2->setChecked(true);});
		hbox->addWidget(e);
		box->addLayout(hbox);
	}

	{
		QButtonGroup *gr = new QButtonGroup(this);
		QHBoxLayout *hbox = new QHBoxLayout();
		hbox->addSpacing(10);
		QRadioButton *r1 = new QRadioButton("Using stdev", window);
		r1->setObjectName("using_stdev");
		r1->setChecked(true);
		hbox->addWidget(r1);
		gr->addButton(r1);
		QRadioButton *r2 = new QRadioButton("or set stdev:", window);
		r2->setObjectName("set_stdev");
		hbox->addWidget(r2);
		gr->addButton(r2);
		QLineEdit *e = new QLineEdit(window);
		e->setObjectName("write_stdev");
		connect(e, &QLineEdit::textChanged,
		        r2, [=]() {r2->setChecked(true);});
		hbox->addWidget(e);
		box->addLayout(hbox);
	}

	{
		QRadioButton *r = new QRadioButton("By unique group", window);
		r->setObjectName("by_unique_group");
		group->addButton(r);
		box->addWidget(r);
	}

	{
		QPushButton *b = new QPushButton("Colour", window);
		connect(b, &QPushButton::clicked, this, &ColourOptions::colour);
		box->addWidget(b);
	}

	setCentralWidget(window);
}

void ColourOptions::colour()
{
	colourByScale();
}

void ColourOptions::colourByScale()
{
	QCheckBox *takeLogObj = findChild<QCheckBox *>("take_log");
	QRadioButton *usingMean = findChild<QRadioButton *>("using_mean");
	QRadioButton *usingStdev = findChild<QRadioButton *>("using_stdev");
	
	bool takeLog = takeLogObj->isChecked();
	bool useMean = usingMean->isChecked();
	bool useStdev = usingStdev->isChecked();
	
	double otherMean = NAN;
	double otherStdev = NAN;
	
	if (!useMean)
	{
		QLineEdit *mean = findChild<QLineEdit *>("write_mean");
		otherMean = atof(mean->text().toStdString().c_str());
	}
	
	if (!useStdev)
	{
		QLineEdit *stdev = findChild<QLineEdit *>("write_stdev");
		otherStdev = atof(stdev->text().toStdString().c_str());
	}
	
	std::cout << useMean << " " << useStdev << std::endl;
	std::cout << otherMean << " " << otherStdev << std::endl;
	_m->colourByScale(takeLog, otherMean, otherStdev);
	
	hide();
	deleteLater();
}
