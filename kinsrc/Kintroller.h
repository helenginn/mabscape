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

#ifndef __abmap__Kintroller__
#define __abmap__Kintroller__

#include <QObject>
#include <map>

class KineticView;
class CurveView;
class Curve;
class QTreeWidget;

typedef struct
{
	double min;
	double max;
} Region;

class Kintroller : public QObject
{
Q_OBJECT
public:
	Kintroller();
	
	void setCommandLineArgs(int argc, char *argv[]);
	void setView(KineticView *view);
	void collectResult(std::string result);
	
	static Kintroller *theKintroller()
	{
		return _kintroller;
	}
public slots:
	void run();
private:
	bool getNicknameArguments(std::string line, std::string *nn,
	                          std::vector<std::string> *args);
	bool makeRegion(std::string line);
	bool makeBuffer(std::string line);
	bool makeOnOff(std::string line);
	bool makeCompetition(std::string line);
	bool processLine(std::string line);
	bool processCurve(Curve *c);

	Curve *makeCurve(std::string nickname, std::string filename,
	                 std::string region1, std::string region2);

	std::vector<std::string> _args;

	std::map<std::string, Region> _regions;
	std::map<std::string, Curve *> _curves;

	QTreeWidget *_tree;
	KineticView *_view;
	CurveView *_cView;
	std::string _resultFile;
	
	bool _dryRun;
	static Kintroller *_kintroller;
};

#endif
