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

#include "Bound.h"
#include "Antibody.h"
#include <hcsrc/Blast.h>
#include <hcsrc/FileReader.h>
#include <cmath>

Antibody::Antibody(Bound *bound, std::string haa, std::string laa)
{
	_bound = bound;
	_haa = haa;
	_laa = laa;
	_sequence = true;
}

std::string Antibody::name()
{
	return _bound->name();
}

double Antibody::compareWithAntibody(Antibody *other, bool heavy)
{
	return compareSequences(other, heavy);
}

double Antibody::compareSequences(Antibody *other, bool heavy)
{
	std::string ohaa = other->_haa;
	std::string olaa = other->_laa;
	
	int muts, dels;
	int size = 0;
	
	if (heavy)
	{
		compare_sequences(_haa, ohaa, &muts, &dels);
		size = heavyLength();
	}
	else
	{
		compare_sequences(_laa, olaa, &muts, &dels);
		size = lightLength();
	}

	double distance = muts / (double)(0.5 * size);

	double score = exp(-(distance * distance));

	return score;
}
