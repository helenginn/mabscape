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

#ifndef __abmap__KineticView__
#define __abmap__KineticView__

#include <QMainWindow>

class CurveView;
class Inspector;
class QTreeWidget;

class KineticView : public QMainWindow
{
Q_OBJECT
public:
	KineticView(QWidget *parent);

	void addCurve(std::string filename);
	void makeMenu();
public slots:
	void grabCSV();
	void itemSelectionChanged();
	void transferRegions();
protected:
	virtual void resizeEvent(QResizeEvent *);
private:
	Inspector *_ins;
	QTreeWidget *_tree;
	CurveView *_cView;

};

#endif
