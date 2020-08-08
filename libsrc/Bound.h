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
#include "SlipObjFile.h"
#include "Icosahedron.h"

class Structure;
class SlipGL;
class Data;
class RefinementStrategy;

class Bound : public Icosahedron
{
public:
	Bound(std::string filename);

	void snapToObject(SlipObject *obj);
	void jiggleOnSurface(SlipObject *obj);
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
	
	static void updateOnRender(bool render)
	{
		_updateOnRender = render;
	}
	
	void setStructure(Structure *s)
	{
		_structure = s;
	}
	
	static double getRadius()
	{
		return _radius;
	}
	
	void addToStrategy(RefinementStrategy *str);
	
	static double getPosX(void *object)
	{
		if (_updateOnRender)
		{
			Bound *b = static_cast<Bound *>(object);
			std::lock_guard<std::mutex> l(b->_mutex);
			return static_cast<Bound *>(object)->_realPosition.x;
		}

		return static_cast<Bound *>(object)->_realPosition.x;
	}
	
	static double getPosY(void *object)
	{
		if (_updateOnRender)
		{
			Bound *b = static_cast<Bound *>(object);
			std::lock_guard<std::mutex> l(b->_mutex);
			return static_cast<Bound *>(object)->_realPosition.y;
		}

		return static_cast<Bound *>(object)->_realPosition.y;
	}

	static double getPosZ(void *object)
	{
		if (_updateOnRender)
		{
			Bound *b = static_cast<Bound *>(object);
			std::lock_guard<std::mutex> l(b->_mutex);
			return static_cast<Bound *>(object)->_realPosition.z;
		}
		return static_cast<Bound *>(object)->_realPosition.z;
	}

	static void setPosX(void *object, double x)
	{
		if (_updateOnRender)
		{
			Bound *b = static_cast<Bound *>(object);
			std::lock_guard<std::mutex> l(b->_mutex);
			static_cast<Bound *>(object)->_realPosition.x = x;
		}
		static_cast<Bound *>(object)->_realPosition.x = x;
	}

	static void setPosY(void *object, double y)
	{
		if (_updateOnRender)
		{
			Bound *b = static_cast<Bound *>(object);
			std::lock_guard<std::mutex> l(b->_mutex);
			static_cast<Bound *>(object)->_realPosition.y = y;
		}
		static_cast<Bound *>(object)->_realPosition.y = y;
	}

	static void setPosZ(void *object, double z)
	{
		if (_updateOnRender)
		{
			Bound *b = static_cast<Bound *>(object);
			std::lock_guard<std::mutex> l(b->_mutex);
			static_cast<Bound *>(object)->_realPosition.z = z;
		}
		static_cast<Bound *>(object)->_realPosition.z = z;
	}
	
	vec3 getWorkingPosition();

	void setRealPosition(vec3 real);
	double scoreWithOther(Bound *other);
	double carefulScoreWithOther(Bound *other);
	
	void setSpecial(bool special);
	void colourFixed();
	
	bool isSpecial()
	{
		return _special;
	}
private:
	double percentageCloudInOther(Bound *b);
	void cloud(double totalPoints);
	void filterCloud();

	std::vector<vec3> _pointCloud;
	std::vector<vec3> _viableCloud;

	std::mutex _mutex;
	Structure *_structure;
	vec3 _realPosition;
	vec3 _nearestNorm;
	static double _radius;
	bool _snapping;
	bool _fixed;
	bool _special;
	static bool _updateOnRender;

};

#endif
