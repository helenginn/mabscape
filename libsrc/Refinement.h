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

#ifndef __abmap__Refinement__
#define __abmap__Refinement__

#include <QObject>
#include <libsrc/Converter.h>

class Experiment;
class Data;
class Bound;

typedef enum
{
	TargetLeastSquares,
	TargetCorrelation,
	TargetBoth
} Target;

class Refinement : public QObject
{
Q_OBJECT
public:
	Refinement(Experiment *exp);

	static double getScore(void *object)
	{
		return static_cast<Refinement *>(object)->score();
	}

	static double getPartialScore(void *object, void *bound)
	{
		Bound *bi = static_cast<Bound *>(bound);
		return static_cast<Refinement *>(object)->partialScore(bi);
	}
	
	static void chooseTarget(Target t)
	{
		_target = t;
	}
	
	static Target currentTarget()
	{
		return _target;
	}
	
	void setFixedOnly(bool only)
	{
		_fixedOnly = only;
	}
	
	void setRandomiseFirst(bool rand)
	{
		_randomiseFirst = rand;
	}
	
	static bool relocatingFliers()
	{
		return _relocateFliers;
	}
	
	static void setRelocateFliers(bool relocate)
	{
		_relocateFliers = relocate;
	}

	void recolourByScore();
	void pause(bool p)
	{
		_pause = p;
	}
	
	static double compareBinders(void *obj, Parameter &p1, Parameter &p2);
signals:
	void failed();
	void resultReady();
public slots:
	void refine();
protected:
private:
	Data *_data;
	double score();
	double partialScore(Bound *bi);
	double _cycleNum;
	bool _fixedOnly;
	bool _pause;
	bool _randomiseFirst;

	static bool _relocateFliers;
	static Target _target;
	static Target _currTarg;
	Experiment *_experiment;

};

#endif
