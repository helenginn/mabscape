// breathalyser
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

#ifndef __breathalyser__arrow__
#define __breathalyser__arrow__

#include <h3dsrc/SlipObject.h>

class Bound;

class Arrow : public SlipObject
{
public:
	Arrow(Bound *parent);
	
	void setEnds(vec3 start, vec3 end)
	{
		_start = start;
		_end = end;
	}
	
	vec3 start()
	{
		return _start;
	}
	
	vec3 end()
	{
		return _end;
	}

	void populate();
private:
	vec3 _start;
	vec3 _end;
	Bound *_bound;

};

#endif
