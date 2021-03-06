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
#include <c4xsrc/AveCSV.h>
#include <iostream>
#include <cmath>
#include <hcsrc/FileReader.h>

std::map<std::string, MapStringDouble> Data::_relationships;
std::map<std::string, MapStringDouble> Data::_finalValues;

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
			std::cout << "Wrong number of terms, skipping: " << std::endl;
			std::cout << lines[i] << std::endl << std::endl;
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
		
		if (val < -0.5) val = -0.5;
		if (val > 1.5) val = 1.5;
		
		for (size_t j = 0; j < components.size(); j++)
		{
			std::string id0 = components[0];
			std::string id1 = components[1];
			
			if (id0 == id1)
			{
				continue;
			}
			
			if (_idCounts.count(id0) == 0)
			{
				_idCounts[id0] = 1; 
			}
			else 
			{ 
				_idCounts[id0]++;
			}

			if (_idCounts.count(id1) == 0) 
			{
				_idCounts[id1] = 1;
			}
			else 
			{ 
				_idCounts[id1]++; 
			}
			
			_relationships[id0][id1] = val;
		}
	}
	
	
	for (std::map<std::string, int>::iterator it = _idCounts.begin(); 
	     it != _idCounts.end(); it++)
	{
		if (std::find(_ids.begin(), _ids.end(), it->first) == _ids.end())
		{
			_ids.push_back(it->first);
		}
	}

	for (size_t i = 0; i < _ids.size(); i++)
	{
		for (size_t j = 0; j < _ids.size(); j++)
		{
			double val = findValueFor(_ids[i], _ids[j]);
			_finalValues[_ids[i]][_ids[j]] = val;
		}
	}
	
	std::cout << "Loaded " << _ids.size() << " binders." << std::endl;
	
//	normalise(true);
//	normalise(false);
}

void Data::normalise(bool higher)
{
	const int topTarget = 4;
	MapStringDouble scales;

	for (size_t i = 0; i < _ids.size(); i++)
	{
		MapStringDouble ab = _relationships[_ids[i]];
		
		double *top = (double *)malloc(sizeof(double) * topTarget);
		int total = 0;
		
		for (int j = 0; j < topTarget; j++)
		{
			top[j] = higher ? 0 : 1;
		}
		
		for (size_t j = 0; j < _ids.size(); j++)
		{
			double candidate = valueFor(_ids[i], _ids[j]);
			
			if (candidate != candidate)
			{
				continue;
			}

			if (candidate > 1.)
			{
				candidate = 1;
			}

			if (candidate < 0.)
			{
				candidate = 0;
			}

			_relationships[_ids[i]][_ids[j]] = candidate;
			
			for (int l = 0; l < topTarget - 1; l++)
			{
				bool comp = (higher ? candidate >= top[l] :
				             candidate <= top[l]);

				if (comp)
				{
					total++;
					for (int k = topTarget - 2; k >= l; k--)
					{
						top[k + 1] = top[k];
					}

					top[l] = candidate;
					break;
				}
			}
		}
		
		if (total > topTarget)
		{
			total = topTarget;
		}
		
		if (total == 0)
		{
			continue;
		}

		std::cout << _ids[i] << " " << higher;
		
		double sum = 0;
		for (int k = 0; k < topTarget; k++)
		{
			std::cout << ", " << top[k];
			sum += top[k];
		}
		
		sum /= (double)total;
		std::cout << " " << sum << std::endl;
		scales[_ids[i]] = sum;
		
		free(top);
	}

	for (size_t i = 0; i < _ids.size(); i++)
	{
		if (scales.count(_ids[i]) == 0)
		{
			continue;
		}
		
		double sum = scales[_ids[i]];

		for (size_t j = 0; j < _ids.size(); j++)
		{
			if (_relationships[_ids[i]].count(_ids[j]) == 0)
			{
				continue;
			}

			if (higher)
			{
				_relationships[_ids[i]][_ids[j]] /= sum;
			}
			else
			{
				double val = _relationships[_ids[i]][_ids[j]];
				double extend = (1 - val) / (1 - sum);
				_relationships[_ids[i]][_ids[j]] = 1 - extend;
				
				if (_ids[i] == "S309")
				{
					std::cout << _ids[j] << " " << val << " " << extend <<
					" " << higher << " " << sum << 
					" = " << 1 - extend << std::endl;
				}
			}
		}
	}
}

double Data::valueFor(std::string i, std::string j)
{
	return _finalValues[i][j];
}

double Data::findValueFor(std::string i, std::string j)
{
	if (_relationships[i].count(j) == 0 &&
	    _relationships[j].count(i) == 0)
	{
		return NAN;
	}
	
	if (_relationships[i].count(j) == 0)
	{
		return _relationships[j][i];
	}
	
	if (_relationships[j].count(i) == 0)
	{
		return _relationships[i][j];
	}
	
	return (_relationships[j][i] + _relationships[i][j]) / 2;
}

void Data::updateCSV(AveCSV *csv)
{
	for (size_t i = 0; i < _ids.size(); i++)
	{
		for (size_t j = 0; j < _ids.size(); j++)
		{
			double val = valueFor(_ids[i], _ids[j]);
			csv->addValue(_ids[i], _ids[j], val);
		}
	}

}

AveCSV *Data::getClusterCSV()
{
	AveCSV *csv = new AveCSV(NULL, "");
	csv->startNewCSV("Data");
	updateCSV(csv);
	return csv;
}
