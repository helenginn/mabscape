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

#ifndef __abmap__antibody__
#define __abmap__antibody__

#include <vector>
#include <string>

class Bound;

class Antibody
{
public:
	Antibody(Bound *bound, std::string haa, std::string laa);

	double compareWithAntibody(Antibody *other, bool heavy);
	
	std::string name();
	size_t heavyLength()
	{
		return _haa.length();
	}

	size_t lightLength()
	{
		return _laa.length();
	}
	
	Bound *bound()
	{
		return _bound;
	}

protected:
	double compareGeneSet(std::vector<std::string> &group1,
	                      std::vector<std::string> &group2,
	                      int maxDrills);

	double compareSequences(Antibody *other, bool heavy);
private:
	Bound *_bound;

	std::string _haa;
	std::string _laa;
	bool _sequence;
};


#endif
