// Slip n Slide
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

#ifndef __Slip__SlipPanel__
#define __Slip__SlipPanel__

#include "SlipObject.h"
#include <vec3.h>

class SlipPanel : public SlipObject
{
public:
	SlipPanel(vec3 x1, vec3 x2, vec3 x3);
	
	void setupVertices(vec3 x1, vec3 x2, vec3 x3);
private:

};

#endif
