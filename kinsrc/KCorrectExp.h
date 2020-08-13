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

#ifndef __abmap__KCorrectExp__
#define __abmap__KCorrectExp__

#include "KOffset.h"
#include "KExpDecay.h"

class KCorrectExp : public KOffset
{
public:
	KCorrectExp();

	void setExponentialDecayModel(KExpDecay *decay)
	{
		_decay = decay;
	}
	
	virtual bool check();
public slots:
	virtual void refineCascade();
protected:
	virtual void addToStrategy(RefinementStrategyPtr str);
	virtual void refine();
	virtual double score();
	void curveChinUp();

	static double getScore(void *object)
	{
		return static_cast<KCorrectExp *>(object)->KCorrectExp::score();
	}

	double bottomEstimate()
	{
		return _yBottom;
	}
private:
	KExpDecay *_decay;

	double _yBottom;

};

#endif
