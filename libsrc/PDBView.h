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

#ifndef __abmap__PDBView__
#define __abmap__PDBView__

#include <Crystal.h>
#include <map>
#include <QMainWindow>

class Bound;
class QPushButton;
class QWidget;
class Experiment;

typedef enum
{
	Antibody,
	Antigen,
	Neither
} AntiType;

class PDBView : public QMainWindow
{
Q_OBJECT
public:
	PDBView(Bound *b);
	
	~PDBView();
	
	void setExperiment(Experiment *e)
	{
		_exp = e;
	}

	bool successful()
	{
		return _success;
	}
private slots:
	void changeChain();
	void calculate();
private:
	void loadInfo();
	bool centroidCheck(std::vector<PolymerPtr> pols);
	void findIntersectingAtoms(std::vector<PolymerPtr> antigens,
	                           std::vector<PolymerPtr> antibodies);
	AtomPtr findMiddlestAtom(std::vector<AtomPtr> &atoms);

	std::vector<QObject *> _bin;
	Bound *_bound;
	CrystalPtr _crystal;
	Experiment *_exp;

	std::map<PolymerPtr, AntiType> _polMap;
	std::vector<AtomPtr> _antigenAtoms;
	std::vector<AtomPtr> _antibodyAtoms;

	bool _success;
};

#endif
