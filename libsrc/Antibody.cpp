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

#include "Antibody.h"
#include "Blast.h"
#include <libsrc/FileReader.h>
#include <cmath>

Antibody::Antibody(std::string name, std::string hv, std::string hj,
                   std::string hd, std::string lv, std::string lj)
{
	_hv = split(hv, '/');
	_hj = split(hj, '/');
	_hd = split(hd, '/');
	_lv = split(lv, '/');
	_lj = split(lj, '/');
	_ab = name;
	_sequence = false;
}

Antibody::Antibody(std::string name, std::string haa, std::string laa)
{
	_ab = name;
	_haa = haa;
	_laa = laa;
	_sequence = true;
}


double compareGenes(std::string one, std::string two, double maxDrills)
{
	std::vector<std::string> one_split = split(one, '*');
	std::vector<std::string> two_split = split(two, '*');
	
	std::string family1 = one_split[0];
	std::string family2 = two_split[0];
	
	std::vector<std::string> one_drill = split(family1, '-');
	std::vector<std::string> two_drill = split(family2, '-');
	
	size_t min = std::min(one_drill.size(), two_drill.size());
	double matches = 0;
	bool giveup = false;
	
	if (maxDrills < min)
	{
		maxDrills = min;
	}
	
	for (size_t i = 0; i < min; i++)
	{
		int d1 = atoi(one_drill[i].c_str());
		int d2 = atoi(two_drill[i].c_str());
		
		if (d1 == d2)
		{
			matches++;
			std::cout << "+" << std::flush;
		}
		else
		{
			std::cout << "-" << std::flush;
			giveup = true;
			break;
		}
	}
	
	if (giveup)
	{
		std::cout << std::endl;
		return matches;
	}

	if (matches >= 0.99 && one_drill.size() == two_drill.size() && 
	    one_split.size() > 1 && two_split.size() > 1)
	{
		if (atoi(one_split[1].c_str()) == atoi(two_split[1].c_str()))
		{
			matches += 1.0;
			std::cout << "*" << std::flush;
		}
	}
	
	std::cout << " " << std::flush;
	
	return matches;
}

double Antibody::compareGeneSet(std::vector<std::string> &group1,
                                std::vector<std::string> &group2,
                                int maxDrills)
{
	double biggest = 0;

	for (size_t i = 0; i < group1.size(); i++)
	{
		for (size_t j = 0; j < group2.size(); j++)
		{
			double match = compareGenes(group1[i], group2[j], maxDrills);
			if (biggest < match)
			{
				biggest = match;
			}
		}
	}

	return biggest;
}

double Antibody::compareWithAntibody(Antibody *other, bool heavy)
{
	if (_sequence)
	{
		return compareSequences(other, heavy);
	}

	double accum = 0;

	accum += compareGeneSet(_hv, other->_hv, 3);
	accum += compareGeneSet(_hj, other->_hj, 1);
	accum += compareGeneSet(_hd, other->_hd, 2);
	accum += compareGeneSet(_lv, other->_lv, 3);
	accum += compareGeneSet(_lj, other->_lj, 1);
	
	std::cout << std::endl;
	accum /= 10;
	
	return accum;
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
		size = _haa.length() + ohaa.length();
	}
	else
	{
		compare_sequences(_laa, olaa, &muts, &dels);
		size = _laa.length() + olaa.length();
	}

	double distance = muts + dels;
	distance /= size / 10;

	double score = exp(-(distance * distance));

	return score;
}
