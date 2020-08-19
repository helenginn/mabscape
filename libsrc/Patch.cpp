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

#include "Patch.h"
#include <iostream>
#include "Structure.h"
#include "shaders/vProj.h"
#include "shaders/fProj.h"

Patch::Patch(Structure *s)
{
	_s = s;
	_distance = 15;
	_centre = empty_vec3();
	_bound = empty_vec3();
	_vString = Patch_vsh();
	_fString = Patch_fsh();
	_patchMat = make_mat4x4();
	copyFrom(_s);
}

void Patch::project()
{
	vec3 o = centroid();
	_bound = _centre;

	_dir = vec3_subtract_vec3(_bound, o);
	Vertex *v = _s->nearestVertexPtr(_centre, true);
	_dir = vec_from_pos(v->normal);
	vec3_set_length(&_dir, -_distance);

	vec3_subtract_from_vec3(&_centre, _dir);
	vec3 tmp = _dir;
	vec3_mult(&tmp, -1);
	vec3_mult(&_dir, 2.5);

	mat3x3 proj = mat3x3_ortho_axes(tmp);
	double det = mat3x3_determinant(proj);
	if (det < 0)
	{
		std::cout << det << std::endl;
		mat3x3_scale(&proj, -1, 1, 1);
	}

	mat3x3 trans = mat3x3_transpose(proj);
	_patchMat = mat4x4_from_rot_trans(trans, empty_vec3());
	std::cout << mat4x4_desc(_patchMat) << std::endl;
	
	std::cout << "Centre: " << vec3_desc(_centre) << std::endl;
	std::cout << "Dir: " << vec3_desc(_dir) << std::endl;
	
	double w = 1;
	vec3 test = mat4x4_mult_vec3(_patchMat, _dir, &w);
	test = mat4x4_mult_vec3(_patchMat, _dir, &w);
	std::cout << "Test: " << vec3_desc(test) << std::endl;
	
	makeLabels();
}

void Patch::makeLabels()
{
	for (size_t i = 0; i < _s->litResidueCount(); i++)
	{
		int residue = _s->litResidue(i);
		std::string resName = _s->residueName(residue);
		
		QLabel *l = new QLabel(resName.c_str(), _pView);
		l->setStyleSheet("QLabel { color : white; }");
		l->setGeometry(0, 0, 120, 40);
		l->setAlignment(Qt::AlignVCenter);
		l->setAlignment(Qt::AlignHCenter);
		l->show();
		_labels[residue] = l;
	}
	
	QLabel *l = new QLabel(_title.c_str(), _pView);
	l->setGeometry(30, 30, 200, 60);
	l->setAlignment(Qt::AlignVCenter);
	l->setAlignment(Qt::AlignHCenter);
	QFont font = l->font();
	font.setPointSize(24);
	l->setFont(font);
	l->setStyleSheet("QLabel { background-color: white; "\
	                 "color : navy; border : 3px solid navy; }");
	l->show();
}

void Patch::updatePositions()
{
	std::map<int, QLabel *>::iterator it;
	mat3x3 rot = mat4x4_get_rot(_model);
	rot = mat3x3_transpose(rot);
	
	for (it = _labels.begin(); it != _labels.end(); it++)
	{
		double w = 1;
		vec3 pos = _s->residuePos(it->first);
		vec3_subtract_from_vec3(&pos, _centre);
		double dot = vec3_dot_vec3(_dir, _dir);
		dot /= vec3_dot_vec3( pos, _dir);
		vec3_mult(&pos, dot);

		pos = mat3x3_mult_vec(rot, pos);
		pos = mat4x4_mult_vec3(_patchMat, pos, &w);
		pos = mat4x4_mult_vec3(_proj, pos, &w);
		vec3_mult(&pos, 1 / w);

		pos.x = pos.x / 2 + 0.5;
		pos.y = -pos.y / 2 + 0.5;
		pos.x *= _pView->width();
		pos.y *= _pView->height();

		QLabel *l = it->second;
		l->setGeometry(pos.x - l->width() / 2, pos.y - l->height() / 2,
		               l->width(), l->height());
	}
}

void Patch::zoom(double val)
{
	vec3 bit = _dir;
	vec3_mult(&bit, val);
	vec3_subtract_from_vec3(&_dir, bit);
	_centre = vec3_subtract_vec3(_bound, _dir);
}

void Patch::extraUniforms()
{
	updatePositions();
	const char *uniform_name = "centre";
	_uCentre = glGetUniformLocation(_program, uniform_name);
	glUniform3f(_uCentre, _centre.x, _centre.y, _centre.z);

	uniform_name = "direction";
	_uDir = glGetUniformLocation(_program, uniform_name);
	glUniform3f(_uDir, _dir.x, _dir.y, _dir.z);

	uniform_name = "patchmat";
	_uMat = glGetUniformLocation(_program, uniform_name);
	glUniformMatrix4fv(_uMat, 1, GL_FALSE, &_patchMat.vals[0]);
}

void Patch::rotated(double x, double y)
{
	std::cout << x << " " << y << std::endl;
	mat3x3 rot = mat3x3_rotate(x, y, 0);
	mat4x4 rot4 = mat4x4_from_rot_trans(rot, empty_vec3());

	_dir = make_vec3(_patchMat.vals[2], _patchMat.vals[6],
	                 _patchMat.vals[10]);
	_patchMat = mat4x4_mult_mat4x4(rot4, _patchMat);
}

void Patch::panned(double x, double y)
{
	double l = vec3_length(_dir);
	vec3 xDir = make_vec3(_patchMat.vals[0], _patchMat.vals[4],
	                      _patchMat.vals[8]);
	vec3 yDir = make_vec3(_patchMat.vals[1], _patchMat.vals[5],
	                      _patchMat.vals[9]);
	mat3x3 rot = mat4x4_get_rot(_model);
//	rot = mat3x3_transpose(rot);
//	mat3x3_mult_vec(rot, &xDir);
//	mat3x3_mult_vec(rot, &yDir);
	
	vec3_mult(&xDir, l * x);
	vec3_mult(&yDir, l * y);
	
	vec3_add_to_vec3(&_centre, xDir);
	vec3_add_to_vec3(&_centre, yDir);
	vec3_add_to_vec3(&_bound, xDir);
	vec3_add_to_vec3(&_bound, yDir);
}

void Patch::render(SlipGL *gl)
{
	reorderIndices();
	SlipObject::render(gl);
}
