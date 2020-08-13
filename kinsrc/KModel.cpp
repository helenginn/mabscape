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

#include "KModel.h"
#include <iostream>

KModel::KModel() : Curve(NULL)
{
	_parent = NULL;
}

void KModel::setCurve(Curve *c)
{
	_parent = c;
	_xs = c->xs();
	_ys.resize(c->xs().size());
}

void KModel::refine()
{
	_anys.clear();
}

void KModel::refineThenDone()
{
	refineCascade();
	emit done();
}

double KModel::score()
{
	_mut.lock();
	populateYs();
	double sum = 0;

	for (size_t i = 0; i < _ys.size(); i++)
	{
		double diff = _ys[i] - curve()->ys()[i];
		
		if (diff == diff)
		{
			sum += diff * diff;
		}
	}

	_mut.unlock();
//	std::unique_lock<std::mutex> lck(_mut);
//	emit changedFit();
//	_cv.wait(lck);

	return sum;
}

QPen KModel::getPen()
{
	return QPen(QColor(100, 100, 255));
}
