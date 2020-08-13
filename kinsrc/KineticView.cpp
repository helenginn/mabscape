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

#include "KineticView.h"
#include "Inspector.h"
#include "CurveView.h"
#include "Curve.h"
#include <QTreeWidget>
#include <QFileDialog>
#include <QMenuBar>

KineticView::KineticView(QWidget *parent) : QMainWindow(parent)
{
	setGeometry(200, 200, 1000, 600);
	makeMenu();
	int h = menuBar()->height();

	_cView = new CurveView(this);
	_cView->setGeometry(200, h, 500, 500 - h);
	
	_ins = new Inspector(this);
	_ins->setGeometry(700, h, 200, 500 - h);
	_ins->setCurveView(_cView);
	_ins->show();
	
	_tree = new QTreeWidget(this);
	_tree->setGeometry(0, h, 200, 500 - h);
	_tree->setHeaderLabel("Curves");
	_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(_tree, &QTreeWidget::itemSelectionChanged,
	        this, &KineticView::itemSelectionChanged);
	_cView->setCurveTree(_tree);
	_ins->setTree(_tree);
}

void KineticView::makeMenu()
{
	QMenu *mc = menuBar()->addMenu(tr("&Curves"));
	QAction *act = mc->addAction(tr("&Load curves..."));
	connect(act, &QAction::triggered, this, &KineticView::grabCSV);

	act = mc->addAction(tr("&Transfer regions to all curves"));
	connect(act, &QAction::triggered, this, &KineticView::transferRegions);

}

void KineticView::resizeEvent(QResizeEvent *)
{
	int h = menuBar()->height();

	_cView->setGeometry(200, h, width() - 400, height() - h);
	_tree->setGeometry(0, h, 200, height() - h);
	_ins->setGeometry(width() - 200, h, 200, height() - h);
	
}

void KineticView::addCurve(std::string filename)
{
	Curve *c = new Curve(_tree);
	c->setFilename(filename);
	c->loadData();
	_tree->addTopLevelItem(c);

}

void KineticView::grabCSV()
{
	QFileDialog *f = new QFileDialog(this, "Choose curve CSV", 
	                                 "Comma-separated values (*.csv)");
	f->setFileMode(QFileDialog::ExistingFiles);
	f->setOptions(QFileDialog::DontUseNativeDialog);
	f->show();

    QStringList fileNames;

    if (f->exec())
    {
        fileNames = f->selectedFiles();
    }
    
    if (fileNames.size() < 1)
    {
		return;
    }

	f->deleteLater();

	for (int i = 0; i < fileNames.size(); i++)
	{
		addCurve(fileNames[i].toStdString());
	}
}

void KineticView::itemSelectionChanged()
{
	_cView->redraw();
	Curve *c = static_cast<Curve *>(_tree->currentItem());
	_ins->setCurve(c);
}

void KineticView::transferRegions()
{
	Curve *parent = static_cast<Curve *>(_tree->currentItem());
	
	for (int i = 0; i < _tree->topLevelItemCount(); i++)
	{
		Curve *c = static_cast<Curve *>(_tree->topLevelItem(i));

		c->copyRegions(parent);
	}
}
