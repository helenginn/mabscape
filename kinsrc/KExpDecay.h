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

#ifndef __abmap__KExpDecay__
#define __abmap__KExpDecay__

#include "KOffset.h"

class KExpDecay : public KOffset
{
public:
	KExpDecay();
	virtual bool check();
	virtual void populateYs();
	
	double getExponent()
	{
		return _exponent;
	}
	
	double getStart()
	{
		return _start;
	}
	
	double getYOffset()
	{
		return _yOffset;
	}
public slots:
	virtual void refineCascade();
protected:
	virtual void refine();
	
	void addToStrategy(RefinementStrategyPtr str);
private:
	double _yOffset;
	double _start;
	double _exponent;
};

#endif
