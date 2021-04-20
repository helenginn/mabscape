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

#ifndef __abmap__Squiggle__
#define __abmap__Squiggle__

#include <hcsrc/vec3.h>
#include <h3dsrc/SlipObject.h>

class Result;

class Squiggle : public SlipObject
{
public:
	Squiggle();

	void setPositions(std::vector<vec3> poz, 
	                  std::vector<Result *> results);
	void clear();
	std::vector<Result *> findResultsBetween(double x1, double y1,
	                                         double x2, double y2);
private:
	std::vector<Result *> _results;

};

#endif
