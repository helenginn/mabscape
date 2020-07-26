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
	
	const double scale = 0.01;
	_model = make_mat4x4();
	mat4x4_mult_scalar(&_model, scale);
	
	addPanel();
}

void SlipGL::addPanel()
{
	vec3 x1 = make_vec3(-1.04, -0.04, -69.6);
	vec3 x2 = make_vec3(18.75, 72.87, -31.63);
	vec3 x3 = make_vec3(-6.98, 69.08, -27.08);
	SlipPanel *spanel = new SlipPanel(x1, x2, x3);
	_panels.push_back(spanel);
}

void SlipGL::preparePanels(int n)
{
	_panels.reserve(n);
}

void SlipGL::panned(double x, double y)
{

}

void SlipGL::draggedLeftMouse(double x, double y)
{
	x /= MOUSE_SENSITIVITY;
	y /= MOUSE_SENSITIVITY;

	mat4x4 mat = make_mat4x4();
	mat4x4_rotate(&mat, y, x, 0);
	_model = mat4x4_mult_mat4x4(mat, _model);
	
	update();
}

void SlipGL::draggedRightMouse(double x, double y)
{


}
void SlipGL::initialisePrograms()
{
	for (unsigned int i = 0; i < _panels.size(); i++)
	{
		_panels[i]->initialisePrograms();
	}
}

void SlipGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (unsigned int i = 0; i < _panels.size(); i++)
	{
		_panels[i]->initialisePrograms();
		_panels[i]->render(this);
	}
}
