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

class Antibody
{
public:
	Antibody(std::string name, std::string hv, std::string hj,
	         std::string hd, std::string lv, std::string lj);

	double compareWithAntibody(Antibody *other);
	
	std::string name()
	{
		return _ab;
	}
protected:
	double compareGeneSet(std::vector<std::string> &group1,
	                      std::vector<std::string> &group2,
	                      int maxDrills);
private:
	std::string _ab;
	std::vector<std::string> _hv;
	std::vector<std::string> _hj;
	std::vector<std::string> _hd;
	std::vector<std::string> _lv;
	std::vector<std::string> _lj;

};


#endif
