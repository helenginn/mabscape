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
#include <map>
#include "SlipObject.h"

class MtzFile;
class QPushButton;
class Result;
class Screen;
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
	
	void setClusterScreen(Screen *screen)
	{
		_screen = screen;
	}

	void highlightBound(Bound *bi);
	
	virtual void render(SlipGL *gl);
	void writeResultsToFile(std::string filename, bool all = false);
	void abPatchwork(std::string id);
private slots:
	void currentItemChanged(QTreeWidgetItem *current, 
	                        QTreeWidgetItem *prev);
	void clear();
	void itemSelectionChanged();
	void makeCluster();
	void readResults();
	void writeResults();
	void cluster4xChanged();
	void patchworkArt();
	void undoArt();
	void summariseBounds();
private:
	void makeExplorer();
	void updateTitle();
	std::map<MtzFile *, Result *> _connections;
	Squiggle *_squiggle;

	Screen *_screen;
	Experiment *_experiment;
	QTreeWidget *_widget;
	QPushButton *_toCluster4x;
	QPushButton *_fetchCluster;
	QPushButton *_writeResults;
	QPushButton *_readResults;
	QPushButton *_clearClusters;
	QPushButton *_patchwork;
	QPushButton *_summarise;
	QPushButton *_undoArt;
};

#endif
