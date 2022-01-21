// mabscape
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

#ifndef __Cone__Cone__
#define __Cone__Cone__

#include <h3dsrc/SlipObject.h>

class Cone : public SlipObject
{
public:
	Cone(vec3 dir, double height);

	void addCircle(vec3 centre, std::vector<vec3> &circle);
	void addCylinderIndices(size_t num);
private:

};

#endif
