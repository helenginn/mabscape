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

#include "Data.h"
#include <iostream>
#include <cmath>
#include <FileReader.h>

std::map<std::string, MapStringDouble> Data::_relationships;

Data::Data(std::string filename)
{
	_filename = filename;
	load();
}

void Data::load()
{
	std::cout << "Should load " << _filename << std::endl;

	std::string contents = get_file_contents(_filename);
	
	std::vector<std::string> lines = split(contents, '\n');
	
	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> components = split(lines[i], ',');
		
		if (components.size() != 3)
		{
			std::cout << "Wrong number of terms, skipping. " << std::endl;
			continue;
		}

		char *pos = NULL;
		double val = strtod(&components[2][0], &pos);

		if (pos == &(components[2][0]))
		{
			if (i == 0)
			{
				std::cout << "First line appears to be header." << std::endl;
			}
			else
			{
				std::cout << "Final value of line " << i << " is not "
				"a number?" << std::endl;
			}

			continue;
		}
		
		if (val < 0) val = 0;
		if (val > 1) val = 1;
		
		for (size_t j = 0; j < components.size(); j++)
		{
			std::string id0 = components[0];
			_idCounts[id0]++;
			std::string id1 = components[1];
			_idCounts[id1]++;
			
			_relationships[id0][id1] = val;
		}
	}
	
	for (std::map<std::string, int>::iterator it = _idCounts.begin(); 
	     it != _idCounts.end(); it++)
	{
		_ids.push_back(it->first);
	}
	
	std::cout << "Loaded " << _ids.size() << " binders." << std::endl;
}

double Data::valueFor(std::string i, std::string j)
{
	double val1 = _relationships[i][j];
	double val2 = _relationships[j][i];
	
	if (val1 != val1 && val2 != val2)
	{
		return nan(" ");
	}
	
	if (fabs(val1) < 1e-6 && fabs(val2) < 1e-6)
	{
		return nan(" ");
	}
	
	if (fabs(val1) < 1e-6)
	{
		return val2;
	}
	
	if (fabs(val2) < 1e-6)
	{
		return val1;
	}
	
	return std::min(val2, val1);
}

