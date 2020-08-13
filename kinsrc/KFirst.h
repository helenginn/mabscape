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

#ifndef __abmap__KFirst__
#define __abmap__KFirst__

#include "KLigOnOff.h"

class KFirst : public KLigOnOff
{
public:
	KFirst();

	virtual void populateYs();
	
	void setFirstModel(KLigOnOff *l)
	{
		_first = l;
	}
	
public slots:
	virtual void refineCascade();
protected:
	void subtractOff();
	virtual void addToStrategy(RefinementStrategyPtr str);
	virtual void refine();
	KLigOnOff *_first;
private:
	double _beginVal;
	bool _hideSecond;
};

#endif
