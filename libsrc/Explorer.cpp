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
#include "SurfaceView.h"
#include "Result.h"
#include "Bound.h"
#include "Squiggle.h"
#include <iostream>
#include <fstream>
#include <MtzFFT.h>
#include <MtzFile.h>
#include <Group.h>
#include <FileReader.h>
#include <AveCSV.h>
#include <Screen.h>
#include <ClusterList.h>
#include <QPushButton>
#include <QFileDialog>

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
	
	int top = 40;

	_toCluster4x = new QPushButton("Datasets to cluster4x", this);
	_toCluster4x->setGeometry(250, top, 200, 40);
	_toCluster4x->show();
	top += 40;

	connect(_toCluster4x, &QPushButton::clicked,
	        this, &Explorer::makeCluster);
	
	_fetchCluster = new QPushButton("Datasets from cluster4x", this);
	_fetchCluster->setGeometry(250, top, 200, 40);
	_fetchCluster->show();
	top += 40;


	connect(_fetchCluster, &QPushButton::clicked,
	        this, &Explorer::cluster4xChanged);
	
	_writeResults = new QPushButton("Write selected results", this);
	_writeResults->setGeometry(250, top, 200, 40);
	_writeResults->show();
	top += 40;

	connect(_writeResults, &QPushButton::clicked,
	        this, &Explorer::writeResults);
	
	_readResults = new QPushButton("Read selected results", this);
	_readResults->setGeometry(250, top, 200, 40);
	_readResults->show();
	top += 40;

	connect(_readResults, &QPushButton::clicked,
	        this, &Explorer::readResults);

	_clearClusters = new QPushButton("Clear all results", this);
	_clearClusters->setGeometry(250, top, 200, 40);
	_clearClusters->show();
	top += 40;

	connect(_clearClusters, &QPushButton::clicked,
	        this, &Explorer::clear);

	_patchwork = new QPushButton("Patchwork graphic", this);
	_patchwork->setGeometry(250, top, 200, 40);
	_patchwork->show();
	top += 40;

	connect(_patchwork, &QPushButton::clicked,
	        this, &Explorer::patchworkArt);

	_summarise = new QPushButton("Average/RMSD bounds", this);
	_summarise->setGeometry(250, top, 200, 40);
	_summarise->show();
	top += 40;

	connect(_summarise, &QPushButton::clicked,
	        this, &Explorer::summariseBounds);

	_undoArt = new QPushButton("Clean up", this);
	_undoArt->setGeometry(250, top, 200, 40);
	_undoArt->show();
	top += 40;

	connect(_undoArt, &QPushButton::clicked,
	        this, &Explorer::undoArt);
	
	
	_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	
	_squiggle = new Squiggle();
	_experiment = NULL;
	initializeOpenGLFunctions();
}

void Explorer::clear()
{
	for (int i = 0; i < _widget->topLevelItemCount(); i++)
	{
		QTreeWidgetItem *item = _widget->topLevelItem(i);
		delete item;
	}

	_connections.clear();
	_widget->clear();
}

void Explorer::updateTitle()
{
	QString num = QString::number(_widget->topLevelItemCount());
	setWindowTitle(num + " results");
}

void Explorer::addResults(std::vector<Result *> results)
{
	for (size_t i = 0; i < results.size(); i++)
	{
		_widget->addTopLevelItem(results[i]);
	}

	_widget->sortItems(0, Qt::AscendingOrder);
	updateTitle();
}

void Explorer::currentItemChanged(QTreeWidgetItem *current, 
                                  QTreeWidgetItem *prev)
{
	if (current == NULL)
	{
		return;
	}

	Result *r = static_cast<Result *>(current);
	r->applyPositions();
	_experiment->somethingToCluster4x(false);
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

void Explorer::makeCluster()
{
	if (_screen == NULL)
	{
		_experiment->getView()->launchCluster4x();
	}

	QList<QTreeWidgetItem *> list = _widget->selectedItems();
	AveCSV::setUsingCSV(false);
	
	std::vector<MtzFFTPtr> datasets;
	for (int i = 0; i < list.size(); i++)
	{
		QTreeWidgetItem *item = list[i];
		Result *r = static_cast<Result *>(item);
		MtzFFTPtr fft = r->makeDatasetForCluster4x();
		datasets.push_back(fft);
		_connections[fft->getMtzFile()] = r;
	}
	
	_screen->getList()->makeGroup(datasets);
}

void Explorer::cluster4xChanged()
{
	if (_screen == NULL)
	{
		return;
	}

	_widget->clearSelection();
	QItemSelectionModel *model = _widget->selectionModel();

	for (size_t i = 0; i < _screen->getList()->groupCount(); i++)
	{
		Group *g = _screen->getList()->group(i);

		for (size_t j = 0; j < g->mtzCount(); j++)
		{
			MtzFile *file = g->getMtzFile(j);

			if (!file->isMarked())
			{
				continue;
			}

			Result *r = _connections[file];

			if (r == NULL)
			{
				continue;
			}

			int index = _widget->indexOfTopLevelItem(r);

			if (index >= 0)
			{
				QModelIndex idx = model->model()->index(index, 0);
				model->select(idx, QItemSelectionModel::Rows 
				              | QItemSelectionModel::Select);
			}

			r->setSelected(true);
		}
	}
}

void Explorer::writeResultsToFile(std::string filename, bool all)
{
	QList<QTreeWidgetItem *> list = _widget->selectedItems();
	
	if (all)
	{
		list.clear();
		
		for (int i = 0; i < _widget->topLevelItemCount(); i++)
		{
			list.push_back(_widget->topLevelItem(i));
		}
	}

	std::ofstream file;
	file.open(filename);
	
	for (int i = 0; i < list.size(); i++)
	{
		QTreeWidgetItem *item = list[i];
		Result *r = static_cast<Result *>(item);
		r->writeToStream(file);
	}
	
	file.close();
}

void Explorer::writeResults()
{
	QFileDialog *f = new QFileDialog(this, "Save results as", 
	                                 "");
	f->setFileMode(QFileDialog::AnyFile);
	f->setAcceptMode(QFileDialog::AcceptSave);
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
	std::string filename = fileNames[0].toStdString();
	
	writeResultsToFile(filename);
}

void Explorer::readResults()
{
	QFileDialog *f = new QFileDialog(this, "Choose stream file", 
	                                 "");
	f->setFileMode(QFileDialog::AnyFile);
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
	std::string filename = fileNames[0].toStdString();
	
	std::string contents = get_file_contents(filename);

	size_t firstpos = 0;
	firstpos = contents.find(">>>", firstpos);
	
	while (firstpos != std::string::npos)
	{
		Result *r = new Result();
		r->setExperiment(_experiment);
		r->populateFromString(contents, &firstpos);
		firstpos = contents.find(">>>", firstpos);

		_widget->addTopLevelItem(r);
	}

	_widget->sortItems(0, Qt::AscendingOrder);
	updateTitle();
}

void Explorer::patchworkArt()
{
	undoArt();
	Structure *s = _experiment->structure();
	s->triangulate();

	QList<QTreeWidgetItem *> list = _widget->selectedItems();
	
	for (size_t j = 0; j < _experiment->boundCount(); j++)
	{
		Bound *b = _experiment->bound(j);

		for (int i = 0; i < list.size(); i++)
		{
			QTreeWidgetItem *item = list[i];
			Result *r = static_cast<Result *>(item);
			r->markVerticesAroundBound(s, b);
		}
		
		s->convertExtraToColour();
	}

	for (size_t i = 0; i < _experiment->boundCount(); i++)
	{
		Bound *bi = _experiment->bound(i);
		bi->setAlpha(0.);
	}
}

void Explorer::undoArt()
{
	Structure *s = _experiment->structure();
	s->recolour(0, 0.5, 0.5);

	for (size_t j = 0; j < _experiment->boundCount(); j++)
	{
		Bound *b = _experiment->bound(j);
		double rad = b->averageRadius();
		double resize = 2 / rad;
		b->resize(resize, true);
	}
}

void Explorer::summariseBounds()
{
	QList<QTreeWidgetItem *> list = _widget->selectedItems();
	
	double sqSum = 0;
	double count = 0;
	std::map<Bound *, double> _map;
	
	for (size_t j = 0; j < _experiment->boundCount(); j++)
	{
		Bound *b = _experiment->bound(j);
		
		if (b->isFixed())
		{
			continue;
		}

		vec3 mean = empty_vec3();

		for (int i = 0; i < list.size(); i++)
		{
			QTreeWidgetItem *item = list[i];
			Result *r = static_cast<Result *>(item);
			vec3 v = r->vecForBound(b);
			vec3_add_to_vec3(&mean, v);
		}

		vec3_mult(&mean, 1 / (double)list.size());
		
		double sum = 0;

		for (int i = 0; i < list.size(); i++)
		{
			QTreeWidgetItem *item = list[i];
			Result *r = static_cast<Result *>(item);
			vec3 v = r->vecForBound(b);
			vec3_subtract_from_vec3(&v, mean);
			double l = vec3_sqlength(v);
			sum += l;
		}

		sum /= (double)list.size();
		double rmsd = sqrt(sum);
		
		std::cout << b->name() << "\t" << rmsd << " Å." << std::endl;
		
		b->setRealPosition(mean);
		b->snapToObject(NULL);
		b->updatePositionToReal();
		
		double rad = b->averageRadius();
		double resize = rmsd / rad;
		b->resize(resize, true);
		
		sqSum += rmsd * rmsd;
		_map[b] = rmsd;
		count++;
	}
	
	double stdev = sqrt(sqSum / count);
	
	for (size_t j = 0; j < _experiment->boundCount(); j++)
	{
		Bound *b = _experiment->bound(j);
		
		if (b->isFixed())
		{
			continue;
		}
		double rmsd = _map[b];
		rmsd /= stdev * 2;
		rmsd = 1 - rmsd;
		
		b->setAlpha(rmsd);
	}
}
