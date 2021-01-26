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

#ifndef __abmap__Bound__
#define __abmap__Bound__

#include <mutex>
#include <h3dsrc/SlipObjFile.h>
#include <h3dsrc/Icosahedron.h>

class Structure;
class SlipGL;
class Arrow;
class Data;
class RefinementStrategy;

class Bound : public Icosahedron
{
public:
	Bound(std::string filename);

	double snapToObject(Structure *obj);
	void jiggleOnSurface(Structure *obj);
	void randomlyPositionInRegion(SlipObject *obj);
	void toggleFixPosition();
	void updatePositionToReal();
	
	void setFixed(bool fixed)
	{
		_fixed = fixed;
		colourFixed();
	}
	
	bool isFixed()
	{
		return _fixed;
	}
	
	void setSnapping(bool snapping);
	virtual void render(SlipGL *gl);
	void colourByValue(double stdev = 1);
	void enableElbow();
	
	void setStructure(Structure *s)
	{
		_structure = s;
	}
	
	static double getRadius(void *object = NULL)
	{
		return _radius;
	}
	
	static void setRadius(void *object, double r)
	{
		_radius = r;
	}
	
	void addToStrategy(RefinementStrategy *str, bool elbow = false);
	
	static double getPosX(void *object)
	{

		return static_cast<Bound *>(object)->_realPosition.x;
	}
	
	static double getPosY(void *object)
	{

		return static_cast<Bound *>(object)->_realPosition.y;
	}

	static double getPosZ(void *object)
	{
		return static_cast<Bound *>(object)->_realPosition.z;
	}

	static void setPosX(void *object, double x)
	{
		static_cast<Bound *>(object)->_realPosition.x = x;
	}

	static void setPosY(void *object, double y)
	{
		static_cast<Bound *>(object)->_realPosition.y = y;
	}

	static void setPosZ(void *object, double z)
	{
		static_cast<Bound *>(object)->_realPosition.z = z;
	}

	static void setElbowAngle(void *object, double a)
	{
		static_cast<Bound *>(object)->_elbowAngle = a;
	}
	
	static double getElbowAngle(void *object)
	{
		return static_cast<Bound *>(object)->_elbowAngle;
	}
	
	vec3 getWorkingPosition();
	vec3 getStoredPosition()
	{
		return _realPosition;
	}

	void setRealPosition(vec3 real);
	double scoreWithOther(Bound *other, bool dampen = false);
	double sigmoidalScore(vec3 posi, vec3 posj, double slope = 2, 
	                      double mult = 1);
	
	void randomiseElbow();
	
	void selectMe(bool sel);
	
	void setSpecial(bool special);
	void colourFixed();
	
	bool isSpecial()
	{
		return _special;
	}
	
	void setValue(double val)
	{
		_value = val;
	}
	
	double getValue()
	{
		return _value;
	}
private:
	void redrawElbow();

	std::mutex _mutex;
	Structure *_structure;
	vec3 _realPosition;
	vec3 _nearestNorm;
	static double _radius;
	bool _snapping;
	bool _fixed;
	bool _special;
	
	Arrow *_arrow;

	double _elbowAngle;
	static double _shoulderAngle;
	double _value;
};

#endif
