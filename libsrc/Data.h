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

#ifndef __abmap__Data__
#define __abmap__Data__

#include <string>
#include <map>
#include <vector>

class Bound;
class AveCSV;

typedef std::map<std::string, double> MapStringDouble;

class Data
{
public:
	Data(std::string filename);

	void load();
	
	size_t boundCount()
	{
		return _ids.size();
	}
	
	std::string boundName(int i)
	{
		return _ids[i];
	}
	
	void changeFilename(std::string fn)
	{
		_filename = fn;
	}
	
	AveCSV *getClusterCSV();
	void updateCSV(AveCSV *csv);
	double valueFor(std::string i, std::string j);
	void normalise(bool higher);
private:
	double findValueFor(std::string i, std::string j);
	std::string _filename;

	std::map<std::string, int> _idCounts;
	std::vector<std::string> _ids;
	static std::map<std::string, MapStringDouble> _relationships;
	static std::map<std::string, MapStringDouble> _finalValues;
};

#endif
