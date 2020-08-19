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

#include "PDBView.h"
#include "Bound.h"
#include "Structure.h"
#include <PDBReader.h>
#include <Atom.h>
#include <Crystal.h>
#include <Polymer.h>
#include <Shouter.h>
#include "Experiment.h"
#include <QFileDialog>
#include <sstream>
#include <QVariant>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QMenu>

PDBView::PDBView(Bound *b)
{
	_bound = b;

	QFileDialog *f = new QFileDialog(this, "Choose fix PDB", 
	                                 "Protein Data Bank format (*.pdb)");
	f->setFileMode(QFileDialog::AnyFile);
	f->setOptions(QFileDialog::DontUseNativeDialog);
	f->show();
	_success = false;

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
	CrystalPtr c;
	
	try
	{
		PDBReader pdb = PDBReader();
		pdb.setFilename(filename);
		c = pdb.getCrystal();
	}
	catch (Shouter *s)
	{
		return;
	}
	
	setGeometry(300, 300, 520, 200);
	show();
	_success = true;
	_crystal = c;
	std::string str = "Calculate fixed position for " + _bound->name();
	setWindowTitle(QString::fromStdString(str));
	loadInfo();
}

void PDBView::loadInfo()
{
	int left = 40;
	int top = 40;
	
	QLabel *l = new QLabel("Identify chains (residue number in brackets)", 
	                       this);
	l->setGeometry(left, top, width() - left, 20);
	l->show();
	_bin.push_back(l);
	
	top += 20;

	for (size_t i = 0; i < _crystal->moleculeCount(); i++)
	{
		MoleculePtr mol = _crystal->molecule(i);
		if (!mol->isPolymer())
		{
//			continue;
		}

		PolymerPtr pol = ToPolymerPtr(mol);
		int resnum = pol->monomerCount();
		std::string q = pol->getChainID() + " (" + i_to_str(resnum) + ")";
		QPushButton *b = new QPushButton(QString::fromStdString(q), this);
		void *polptr = &*pol;
		QVariant vPol = QVariant::fromValue(polptr);
		b->setGeometry(left, top, 100, 40);
		b->show();
		left += 110;
		
		QMenu *m = new QMenu(b);
		_bin.push_back(m);
		QAction *a;
		a = m->addAction("Neither");
		a->setProperty("chain", vPol);
		a->setProperty("button", QVariant::fromValue(b));
		_bin.push_back(a);
		connect(a, &QAction::triggered, this, &PDBView::changeChain);
		a = m->addAction("Antibody");
		a->setProperty("chain", vPol);
		a->setProperty("button", QVariant::fromValue(b));
		_bin.push_back(a);
		connect(a, &QAction::triggered, this, &PDBView::changeChain);
		a = m->addAction("Antigen");
		a->setProperty("chain", vPol);
		a->setProperty("button", QVariant::fromValue(b));
		_bin.push_back(a);
		connect(a, &QAction::triggered, this, &PDBView::changeChain);
		
		b->setMenu(m);
		
		if (left > width())
		{
			left = 40;
			top += 50;
		}

		_bin.push_back(b);
	}
	
	left = 40;
	top += 50;
	QPushButton *b = new QPushButton("Calculate", this);
	connect(b, &QPushButton::clicked, this, &PDBView::calculate);
	b->setGeometry(left, top, 150, 40);
	b->show();
	
	top += 40;
	setMinimumHeight(top);
	_bin.push_back(b);
}

PDBView::~PDBView()
{
}

void PDBView::changeChain()
{
	QAction *a = static_cast<QAction *>(QObject::sender());
	void *v = a->property("chain").value<void *>();
	Polymer *polptr = static_cast<Polymer *>(v);

	if (!polptr)
	{
		return;
	}

	PolymerPtr pol = polptr->shared_from_this();

	std::string str = a->text().toStdString();

	QColor c = QColor(Qt::transparent);
	AntiType type = Neither;
	if (str == "Antibody")
	{
		type = Antibody;
		c = QColor(Qt::blue);
	}
	else if (str == "Antigen")
	{
		type = Antigen;
		c = QColor(Qt::yellow);
	}
	
	QPushButton *b = a->property("button").value<QPushButton *>();

	_polMap[pol] = type;
	
	QPalette palette = b->palette();
	palette.setColor(QPalette::Window, c);
	b->setPalette(palette);
}

bool PDBView::centroidCheck(std::vector<PolymerPtr> pols)
{
	AtomGroupPtr grp = AtomGroupPtr(new AtomGroup());
	for (size_t i = 0; i < pols.size(); i++)
	{
		grp->addAtomsFrom(pols[i]);
	}

	vec3 centroid = grp->centroid();
	vec3 compare = _exp->structure()->centroid();
	vec3 diff = vec3_subtract_vec3(compare, centroid);
	
	double length = vec3_length(diff);
	std::cout << "Difference in antigen centroid: " 
	<< length << " Å." << std::endl;
	
	std::cout << vec3_desc(compare) << std::endl;
	std::cout << vec3_desc(centroid) << std::endl;

	if (length > 20.)
	{
		return false;
	}
	
	return true;
}

void PDBView::calculate()
{
	std::map<PolymerPtr, AntiType>::iterator it;
	std::vector<PolymerPtr> _antibodies;
	std::vector<PolymerPtr> _antigens;

	for (it = _polMap.begin(); it != _polMap.end(); it++)
	{
		PolymerPtr pol = it->first;
		AntiType type = it->second;
		
		if (type == Antibody)
		{
			_antibodies.push_back(pol);
		}
		else if (type == Antigen)
		{
			_antigens.push_back(pol);
		}
	}

	bool check = centroidCheck(_antigens);
	
	if (!check)
	{
		QMessageBox msg;
		msg.setText("Calculation failed antigen centroid check (loaded "\
		            "antigen differs by more than 20 Å from antigen "\
		            "defined by PDB. Have you superimposed your "\
		            "antigen and defined it correctly?");
		msg.exec();
		return;
	}
	
	findIntersectingAtoms(_antigens, _antibodies);
	
	std::ostringstream result;
	result << "Found " << _antigenAtoms.size() << " antigen atoms "
	"and " << _antibodyAtoms.size() << " antibody atoms." << std::endl;

	if (_antibodyAtoms.size() == 0 || _antigenAtoms.size() == 0)
	{
		QMessageBox msg;
		msg.setText(QString::fromStdString(result.str()));
		msg.exec();
		return;
	}
	
	AtomPtr ag = findMiddlestAtom(_antigenAtoms);
	AtomPtr ab = findMiddlestAtom(_antibodyAtoms);
	
	result << "Middle interacting antigen atom: " << 
	ag->shortDesc() << std::endl;
	result << "Middle interacting antibody atom: " << 
	ab->shortDesc() << std::endl;
	
	vec3 agpos = ag->getPDBPosition();
	vec3 abpos = ab->getPDBPosition();
	vec3 both = vec3_add_vec3(abpos, agpos);
	vec3_mult(&both, 0.5);

	result << "Calculated midpoint " << vec3_desc(both) << std::endl;
	_bound->setRealPosition(both);
	_bound->snapToObject(NULL);
	_bound->updatePositionToReal();
	_bound->setFixed(true);

	QMessageBox msg;
	msg.setText(QString::fromStdString(result.str()));
	msg.exec();
	
	hide();
	deleteLater();
	return;
}

void PDBView::findIntersectingAtoms(std::vector<PolymerPtr> antigens,
                                    std::vector<PolymerPtr> antibodies)
{
	AtomGroupPtr antigen = AtomGroupPtr(new AtomGroup());
	for (size_t i = 0; i < antigens.size(); i++)
	{
		antigen->addAtomsFrom(antigens[i]);
	}

	AtomGroupPtr antibody = AtomGroupPtr(new AtomGroup());
	for (size_t i = 0; i < antibodies.size(); i++)
	{
		antibody->addAtomsFrom(antibodies[i]);
	}
	
	const double close = 10.;
	
	for (size_t i = 0; i < antigen->atomCount(); i++)
	{
		AtomPtr ag = antigen->atom(i);
		vec3 genpos = ag->getPDBPosition();

		for (size_t j = 0; j < antibody->atomCount(); j++)
		{
			AtomPtr ab = antibody->atom(j);
			vec3 bodypos = ab->getPDBPosition();

			vec3 diff = vec3_subtract_vec3(bodypos, genpos);

			double length = vec3_length(diff);

			if (abs(diff.x) > close || abs(diff.y) > close 
			    || abs(diff.z) > close)
			{
				continue;
			}
			
			if (length > 5)
			{
				continue;
			}

			if (std::find(_antigenAtoms.begin(), 
			              _antigenAtoms.end(), ag) == _antigenAtoms.end())
			{
				_antigenAtoms.push_back(ag);
			}

			if (std::find(_antibodyAtoms.begin(), 
			              _antibodyAtoms.end(), ab) == _antibodyAtoms.end())
			{
				_antibodyAtoms.push_back(ab);
			}
		}
	}
}

AtomPtr PDBView::findMiddlestAtom(std::vector<AtomPtr> &atoms)
{
	double bestScore = FLT_MAX;
	AtomPtr chosen;

	for (size_t i = 0; i < atoms.size(); i++)
	{
		vec3 ipos = atoms[i]->getPDBPosition();

		double sum = 0;
		for (size_t j = 0; j < atoms.size(); j++)
		{
			vec3 jpos = atoms[j]->getPDBPosition();
			vec3_subtract_from_vec3(&jpos, ipos);
			double sq = vec3_sqlength(jpos);

			sum += sq;
		}

		sum /= (double)atoms.size();
		
		if (sum < bestScore)
		{
			bestScore = sum;
			chosen = atoms[i];
		}
	}

	return chosen;
}
