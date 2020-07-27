// SlipGL
// Copyright (C) 2017-2018 Helen Ginn
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

#include "SlipGL.h"
#include "SlipPanel.h"
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWindow>
#include <QTimer>
#include <iostream>

#define MOUSE_SENSITIVITY 500
#define START_Z -64

void SlipGL::initializeGL()
{
	initializeOpenGLFunctions();

	glClearColor(1.0, 1.0, 1.0, 1.0);

//	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initialisePrograms();
}

SlipGL::SlipGL(QWidget *p) : QOpenGLWidget(p)
{
	/*
	_timer = new QTimer();
	_timer->setInterval(1);
	connect(_timer, &QTimer::timeout, this, &SlipGL::update);
	*/

	setGeometry(p->geometry());
	updateProjection();
	setupCamera();
}

void SlipGL::addObject(SlipObject *obj, bool active)
{
	_objects.push_back(obj);
	obj->setModel(_model);
	obj->reorderIndices();
	
	if (active)
	{
		_activeObj = obj;
	}
}

void SlipGL::addPanel()
{
	vec3 x1 = make_vec3(-1.04, -0.04, -69.6);
	vec3 x2 = make_vec3(18.75, 72.87, -31.63);
	vec3 x3 = make_vec3(-6.98, 69.08, -27.08);
	SlipPanel *spanel = new SlipPanel(x1, x2, x3);
	_objects.push_back(spanel);
	_activeObj = spanel;
}

void SlipGL::preparePanels(int n)
{
	_objects.reserve(n);
}

void SlipGL::panned(double x, double y)
{

}

void SlipGL::zoom(float x, float y, float z)
{
	_translation.x += x;
	_translation.y += y;
	_translation.z += z;
	
	_transOnly.x += x;
	_transOnly.y += y;
	_transOnly.z += z;

}

void SlipGL::draggedLeftMouse(double x, double y)
{
	x /= MOUSE_SENSITIVITY;
	y /= MOUSE_SENSITIVITY;

	_camAlpha -= y;
	_camBeta += x;
	_camGamma -= 0;

	updateCamera();
	
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		_objects[i]->setModel(_model);
		_objects[i]->reorderIndices();
	}

	update();
}

void SlipGL::draggedRightMouse(double x, double y)
{
	zoom(0, 0, -y / MOUSE_SENSITIVITY * 10);
	update();
}
void SlipGL::initialisePrograms()
{
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		_objects[i]->initialisePrograms();
	}
}

void SlipGL::paintGL()
{
	updateCamera();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		_objects[i]->initialisePrograms();
		_objects[i]->render(this);
	}
}

void SlipGL::updateCamera()
{
	vec3 centre = _centre;
	centre.x = 0;
	centre.y = 0;
	
	vec3 negCentre = centre;
	vec3_mult(&centre, -1);

	mat4x4 change = make_mat4x4();
	mat4x4_translate(&change, negCentre);
	mat4x4_rotate(&change, _camAlpha, _camBeta, _camGamma);
	mat4x4_translate(&change, centre);

	mat4x4 transMat = make_mat4x4();
	_centre = vec3_add_vec3(_centre, _translation);
	mat4x4_translate(&transMat, _translation);

	mat4x4 tmp = mat4x4_mult_mat4x4(change, transMat);
	_model = mat4x4_mult_mat4x4(_model, tmp);

	_camAlpha = 0; _camBeta = 0; _camGamma = 0;
	_translation = make_vec3(0, 0, 0);
}


void SlipGL::setupCamera()
{
	_translation = make_vec3(0, 0, START_Z);
	_transOnly = make_vec3(0, 0, 0);
	_totalCentroid = make_vec3(0, 0, 0);
	_centre = make_vec3(0, 0, 0);
	_camAlpha = 0;
	_camBeta = 0;
	_camGamma = 0;
	_model = make_mat4x4();

	updateProjection();
	updateCamera();
}


void SlipGL::updateProjection()
{
	zNear = 4;
	zFar = 100;

	double side = 0.5;
	float aspect = (float)height() / (float)width();
	_proj = mat4x4_ortho(side, -side, side * aspect, -side * aspect,
	                     zNear, zFar);
}
