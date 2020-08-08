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

#ifndef __abmap__Result__
#define __abmap__Result__

#include <QTreeWidgetItem>
#include "PositionMap.h"
#include "MtzFFTPtr.h"

class Experiment;
class Structure;

class Result : public QTreeWidgetItem
{
public:
	Result();
	
	void savePositions();
	void applyPositions();
	
	void setExperiment(Experiment *exp)
	{
		_exp = exp;
	}

	void setPositions(PositionMap map)
	{
		_map = map;
	}
	
	void setScore(double score)
	{
		_score = score;
		setText(0, "Score: " + QString::number(_score));
	}
	
	vec3 vecForBound(Bound *b);
	
	double score()
	{
		return _score;
	}
	
	static bool result_is_less_than(Result *a, Result *b)
	{
		return (a->_score < b->_score);

	}

	void populateFromString(std::string contents, size_t *pos);
	void writeToStream(std::ofstream &file);
	void markVerticesAroundBound(Structure *s, Bound *b);

	MtzFFTPtr makeDatasetForCluster4x();
private:
	Experiment *_exp;
	PositionMap _map;
	double _score;

};

#endif
