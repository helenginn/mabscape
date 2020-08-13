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

#ifndef __abmap__KModel__
#define __abmap__KModel__

#include <shared_ptrs.h>
#include "Curve.h"

class KModel : public Curve
{
Q_OBJECT
public:
	KModel();

	void setCurve(Curve *c);
	virtual bool check() = 0;
	virtual void populateYs() = 0;

	Curve *curve()
	{
		return _parent;
	}
public slots:
	virtual void refineCascade() = 0;
	virtual void refineThenDone();
signals:
	void changedFit();
	void done();
protected:
	virtual QPen getPen();
	virtual void addToStrategy(RefinementStrategyPtr str) = 0;
	virtual void refine() = 0;
	static double getScore(void *object)
	{
		return static_cast<KModel *>(object)->KModel::score();
	}
	
	virtual double score();

	std::vector<AnyPtr> _anys;
private:
	Curve *_parent;
};

#endif
