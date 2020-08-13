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

#include "Inspector.h"
#include "CurveView.h"
#include "Curve.h"
#include <iostream>
#include <QComboBox>
#include <QTreeWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QVariant>
#include <QLabel>

Inspector::Inspector(QWidget *w) : QWidget(w)
{
	_region = -1;
	_c = NULL;

}

void Inspector::setCurve(Curve *c)
{
	_c = c;
	if (_c != NULL)
	{
		repopulate();
	}
}

void Inspector::repopulate()
{
	qDeleteAll(children());

	_box = new QComboBox(this);
	_box->setGeometry(10, 20, width() - 20, 40);
	_box->addItem("<< not set >>", QVariant(CurveUnknown));
	if (_c->type() == CurveUnknown)
	{
		_box->setCurrentIndex(_box->count() - 1);
	}
	_box->addItem("On/off curve", QVariant(CurveOnOff));
	if (_c->type() == CurveOnOff)
	{
		_box->setCurrentIndex(_box->count() - 1);
	}
	_box->addItem("Competition curve", QVariant(CurveCompetition));
	if (_c->type() == CurveCompetition)
	{
		_box->setCurrentIndex(_box->count() - 1);
	}
	_box->addItem("Buffer", QVariant(CurveBuffer));
	if (_c->type() == CurveBuffer)
	{
		_box->setCurrentIndex(_box->count() - 1);
	}

	connect(_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [=](int index){ comboChanged(index); });

	_box->show();
	
	_curveMap.clear();
	triageButtons();
}

void Inspector::comboChanged(int idx)
{
	QVariant v = _box->itemData(idx);
	unsigned int ui = v.toUInt();
	CurveType t = (CurveType)ui;
	_c->setType(t);
	
	triageButtons();
}


void Inspector::triageButtons()
{
	QList<QWidget *> subs;
	subs = findChildren<QWidget *>("sub", Qt::FindDirectChildrenOnly);
	
	for (int i = 0; i < subs.size(); i++)
	{
		subs[i]->hide();
		subs[i]->deleteLater();
	}

	CurveType t = _c->type();

	if (t == CurveOnOff)
	{
		onOffButtons();
	}
	else if (t == CurveBuffer)
	{
		bufferButtons();
	}
	else if (t == CurveCompetition)
	{
		competitionButtons();
	}
}

void Inspector::decorateRegionSetter(QPushButton *r, QString str, int reg)
{
	if (_cView->mode() == ModeSetRegion && _region == reg)
	{
		r->setText("Drag " + str + " region");
		r->setStyleSheet("QPushButton {background-color: #FFAAAA; "\
		                   "color: red;}");
	}
	else if (_c->isRegionSet(reg))
	{
		r->setText("Reset " + str + " region");
		r->setStyleSheet("");
	}
	else
	{
		r->setText("Set " + str + " region");
		r->setStyleSheet("QPushButton {background-color: #FFAAAA; "\
		                   "color: red;}");
	}
}

void Inspector::makeRegionSetters(int *top)
{
	for (int i = 0; i < 3; i++)
	{
		if (_regionLabels[i] == "")
		{
			continue;
		}

		_rs[i] = new QPushButton("", this);
		_rs[i]->setGeometry(10, *top, width() - 20, 40);
		_rs[i]->setObjectName("sub");
		*top += 50;
		connect(_rs[i], &QPushButton::clicked, this, &Inspector::setRegion);
		_rs[i]->show();
	}

}

void Inspector::decorateRegionSetters()
{
	for (int i = 0; i < 3; i++)
	{
		if (_regionLabels[i] == "")
		{
			_rs[i] = NULL;
			continue;
		}

		decorateRegionSetter(_rs[i], _regionLabels[i], i);
	}
}

void Inspector::chooseCurveCombo(int *top, QComboBox **boxPtr,
                                 CurveType type, std::string desc)
{
	std::string text = "Choose " + desc + ":";
	QLabel *l = new QLabel(text.c_str(), this);
	l->setGeometry(10, *top, width() - 20, 40);
	l->setObjectName("sub");
	l->show();
	*top += 40;

	*boxPtr = new QComboBox(this);
	(*boxPtr)->setGeometry(10, *top, width() - 20, 40);
	(*boxPtr)->addItem("<< not set >>", QVariant());
	(*boxPtr)->show();
	(*boxPtr)->setProperty("choice", QString::fromStdString(desc));
	(*boxPtr)->setObjectName("sub");

	connect(*boxPtr, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(choiceChanged(int)));
	
	for (int i = 0; i < _tree->topLevelItemCount(); i++)
	{
		Curve *c = static_cast<Curve *>(_tree->topLevelItem(i));
		if (c->hasModel() && c->type() == type)
		{
			QString str = c->text(0);
			(*boxPtr)->addItem(str, QVariant());
			_curveMap[str.toStdString()] = c;
		}
	}
	
	*top += 50;

}

void Inspector::chooseBufferCombo(int *top) 
{
	chooseCurveCombo(top, &_bufferChoice, CurveBuffer, "buffer");
}

void Inspector::choiceChanged(int index)
{
	QComboBox *combo = qobject_cast<QComboBox *>(sender());
	
	QString val = combo->property("choice").value<QString>();
	std::cout << val.toStdString() << std::endl;
	
	if (val.toStdString() == "buffer")
	{
		std::string str = _bufferChoice->itemText(index).toStdString();
		Curve *chosen = _curveMap[str];

		_c->setBuffer(chosen);
	}
	else if (val.toStdString() == "first Ab")
	{
		std::string str = _firstChoice->itemText(index).toStdString();
		Curve *chosen = _curveMap[str];

		_c->setFirstAb(chosen);
	}
	else if (val.toStdString() == "second Ab")
	{
		std::string str = _secondChoice->itemText(index).toStdString();
		Curve *chosen = _curveMap[str];

		_c->setSecondAb(chosen);
	}
}

void Inspector::bufferButtons()
{
	int top = 70;
	_regionLabels[0] = "fall-off";
	_regionLabels[1] = "post-jump";
	_regionLabels[2] = "";
	
	makeRegionSetters(&top);
	decorateRegionSetters();

	makeCalculate(&top);
	makeOriginal(&top);
}

void Inspector::makeCalculate(int *top)
{
	_calculate = new QPushButton("Calculate", this);
	_calculate->setGeometry(10, *top, width() - 20, 40);
	_calculate->setObjectName("sub");
	*top += 50;
	connect(_calculate, &QPushButton::clicked, this, &Inspector::calculate);
	_calculate->show();
}

void Inspector::makeOriginal(int *top)
{

	_original = new QPushButton("Original", this);
	_original->setGeometry(10, *top, width() - 20, 40);
	_original->setObjectName("sub");
	*top += 50;
	connect(_original, &QPushButton::clicked, this, &Inspector::toOriginal);
	_original->show();
}

void Inspector::competitionButtons()
{
	int top = 70;
	_regionLabels[0] = "first Ab";
	_regionLabels[1] = "second Ab";
	_regionLabels[2] = "";
	
	makeRegionSetters(&top);
	decorateRegionSetters();
	chooseBufferCombo(&top);

	chooseCurveCombo(&top, &_firstChoice, CurveOnOff, "first Ab");
	chooseCurveCombo(&top, &_secondChoice, CurveOnOff, "second Ab");

	makeCalculate(&top);
	makeOriginal(&top);
}

void Inspector::onOffButtons()
{
	int top = 70;
	_regionLabels[0] = "on";
	_regionLabels[1] = "off";
	_regionLabels[2] = "";
	
	makeRegionSetters(&top);
	decorateRegionSetters();
	chooseBufferCombo(&top);

	makeCalculate(&top);
	makeOriginal(&top);
}

void Inspector::finishRegion()
{
	double start, end;
	_cView->getRegion(&start, &end);
	_c->setRegion(_region, start, end);
	std::cout << "Region: " << start << " " << end << std::endl;
	_region = -1;
	_cView->setMode(ModeNormal);
	_cView->redraw();
	decorateRegionSetters();
}

void Inspector::setRegion()
{
	if (QObject::sender() == _rs[0])
	{
		_region = 0;
	}
	else if (QObject::sender() == _rs[1])
	{
		_region = 1;
	}
	else if (QObject::sender() == _rs[2])
	{
		_region = 2;
	}
	
	_cView->setMode(ModeSetRegion);
	_cView->setFocus();
	connect(_cView, &CurveView::finishedRegionSet,
	        this, &Inspector::finishRegion, Qt::UniqueConnection);
	decorateRegionSetters();
}

void Inspector::calculate()
{
	_c->calculate();
}

void Inspector::toOriginal()
{
	_c->toOriginal();
}
