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

#ifndef __abmap__Inspector__
#define __abmap__Inspector__

#include <QWidget>
#include "Curve.h"

class CurveView;
class QComboBox;
class QTreeWidget;
class QPushButton;
class QCheckBox;

class Inspector : public QWidget
{
Q_OBJECT
public:
	Inspector(QWidget *w);
	
	void setCurveView(CurveView *v)
	{
		_cView = v;
	}
	
	void setTree(QTreeWidget *tree)
	{
		_tree = tree;
	}
	
	void setCurve(Curve *c);
private slots:
	void comboChanged(int idx);
	void choiceChanged(int index);
	void finishRegion();
	void setRegion();
	void calculate();
	void toOriginal();
private:
	void makeRegionSetters(int *top);
	void makeCalculate(int *top);
	void makeOriginal(int *top);
	void decorateRegionSetters();
	void chooseBufferCombo(int *top) ;
	void chooseCurveCombo(int *top, QComboBox **boxPtr,
	                      CurveType type, std::string desc);
	void decorateRegionSetter(QPushButton *r, QString str, int reg);
	void triageButtons();
	void bufferButtons();
	void onOffButtons();
	void competitionButtons();
	void repopulate();
	Curve *_c;
	CurveView *_cView;
	QComboBox *_box;
	QComboBox *_bufferChoice;
	QComboBox *_firstChoice;
	QComboBox *_secondChoice;
	QPushButton *_rs[3];
	QPushButton *_calculate;
	QPushButton *_original;
	QTreeWidget *_tree;

	QString _regionLabels[3];
	int _region;
	std::map<std::string, Curve *> _curveMap;
};

#endif
