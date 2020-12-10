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

#ifndef __abmap__Patch__
#define __abmap__Patch__

#include <SlipObject.h>
#include "PatchView.h"
#include <QLabel>
#include <vec3.h>
#include <QtGui/qopengl.h>
#include <QtGui/qopenglfunctions.h>

class Structure;

class Patch : public SlipObject
{
public:
	Patch(Structure *s);
	
	void setDistance(double dist)
	{
		_distance = dist;
	}
	
	void setCentre(vec3 c)
	{
		_centre = c;
	}
	
	void setTitle(std::string title)
	{
		_title = title;
	}

	void project();
	void zoom(double val);
	void panned(double x, double y);
	void rotated(double x, double y);
	virtual void render(SlipGL *gl);
	
	void setPatchView(PatchView *view)
	{
		_pView = view;
	}
protected:
	virtual void extraUniforms();
private:
	void makeLabels();
	void updatePositions();
	std::map<int, QLabel *> _labels;
	std::string _title;

	Structure *_s;
	PatchView *_pView;

	vec3 _bound;
	vec3 _centre;
	vec3 _dir;
	mat4x4 _patchMat;
	double _distance;
	GLuint _uMat;
	GLuint _uCentre;
	GLuint _uDir;
};

#endif
