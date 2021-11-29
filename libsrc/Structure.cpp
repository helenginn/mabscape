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

#include "Structure.h"
#include "Glowable_sh.h"

#include <h3dsrc/Mesh.h>
#include <libccp4/csymlib.h>
#include <iostream>
#include <libsrc/PDBReader.h>
#include <libsrc/Monomer.h>
#include <libsrc/Crystal.h>
#include <libsrc/Atom.h>
#include <h3dsrc/shaders/vStructure.h>
#include <h3dsrc/shaders/fStructure.h>

using namespace Helen3D;

Structure::Structure(std::string filename) : SlipObjFile(filename)
{
	_vString = Glowable_vsh();
	_fString = Glowable_fsh();
	resize(1);
	setName("Structure");
	recolour(0.8, 0.3, 0.8);
	_triangulation = 0;
	
	bool collapsed = collapseCommonVertices();
	calculateNormalsAndCheck();
	
	if (collapsed)
	{
		writeObjFile("smaller-" + filename);
	}
}

void Structure::clearExtra()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].extra[0] = 0;
	}

}

void Structure::turtleShell()
{
	AtomMap::iterator it;
	for (it = _atomMap.begin(); it != _atomMap.end(); it++)
	{
		for (size_t i = 0; i < _vertices.size(); i++)
		{
			_vertices[i].extra[0] = 0;
		}

		std::vector<Vertex *> vs = it->second;

		for (size_t i = 0; i < vs.size(); i++)
		{
			vs[i]->extra[0] = 1;
		}

		convertExtraToColour();
	}

}

void Structure::addPDB(std::string filename)
{
	_resNames.clear();
	_resPos.clear();
	_atomMap.clear();
	_singleMap.clear();
	_resiList.clear();

	PDBReader pdb = PDBReader();
	pdb.setFilename(filename);
	_crystal = pdb.getCrystal();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		double closest = FLT_MAX;
		vec3 vtx = vec_from_pos(_vertices[i].pos);
		AtomPtr atom;

		for (size_t j = 0; j < _crystal->atomCount(); j++)
		{
			if (_crystal->atom(j)->isHeteroAtom())
			{
				continue;
			}
			
			vec3 abs = _crystal->atom(j)->getAbsolutePosition();
			vec3 diff = vec3_subtract_vec3(abs, vtx);
			
			double test = vec3_sqlength(diff);
			if (test < closest)
			{
				closest = test;
				atom = _crystal->atom(j);
			}
		}
		
		if (atom && atom->getMonomer())
		{
			int res = atom->getResidueNum();
			std::string type = atom->getMonomer()->getIdentifier();
			std::string str = type + i_to_str(res);
			_resNames[res] = str;
			_atomMap[atom->getResidueNum()].push_back(&_vertices[i]);
			_singleMap[&_vertices[i]] = res;
		}
	}
	
	std::cout << "Total resis: " << _atomMap.size() << std::endl;
	
	AtomMap::iterator it;
	for (it = _atomMap.begin(); it != _atomMap.end(); it++)
	{
		vec3 sum = empty_vec3();
		std::vector<Vertex *> vs = it->second;
		
		for (size_t i = 0; i < vs.size(); i++)
		{
			vec3 pos = vec_from_pos(vs[i]->pos);
			vec3_add_to_vec3(&sum, pos);
		}
		
		vec3_mult(&sum, 1 / (double)vs.size());
		_resPos[it->first] = sum;
	}
	
	turtleShell();
	resultReady();
}

void Structure::markExtraAround(vec3 pos, double reach, bool fillout)
{
	std::vector<int> allResis;

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&v, pos);
		
		if (vec3_length(v) > reach)
		{
			continue;
		}
		
		int resi = _singleMap[&_vertices[i]];
		
		if (std::find(allResis.begin(), allResis.end(), resi) 
		    == allResis.end())
		{
			allResis.push_back(resi);
		}
		
		if (_vertices[i].color[0] < 1e-6 &&
		    _vertices[i].color[1] < 1e-6 &&
		    _vertices[i].color[2] < 1e-6)
		{
			continue;
		}

		_vertices[i].color[0] = 0.4;
		_vertices[i].color[1] = 0.4;
		_vertices[i].color[2] = 1.0;
	}

	if (fillout && _crystal)
	{
		for (size_t i = 0; i < allResis.size(); i++)
		{
			markExtraResidue(allResis[i]);
		}
	}
	
	_resiList = allResis;
}

void Structure::markExtraResidue(int resi)
{
	if (_atomMap.count(resi) == 0)
	{
		return;
	}

	std::vector<Vertex *> vs = _atomMap[resi];

	for (size_t i = 0; i < vs.size(); i++)
	{
		if (vs[i]->color[0] < 1e-6 &&
		    vs[i]->color[1] < 1e-6 &&
		    vs[i]->color[2] < 1e-6)
		{
			continue;
		}

//		vs[i]->color[0] = 0.4;
//		vs[i]->color[1] = 0.4;
//		vs[i]->color[2] = 1.0;
		vs[i]->extra[0] = 1.0;
	}
}

void Structure::convertExtraToColour(double red, double green, double blue)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (_vertices[i].extra[0] < 0.5)
		{
			continue;
		}

		bool variagated = false;
		for (size_t j = 0; j < _indices.size(); j += 3)
		{
			if (_indices[j] == i || _indices[j+1] == i || _indices[j+2] == i)
			{
				if (_vertices[_indices[j+0]].extra[0] < 0.5 ||
				    _vertices[_indices[j+1]].extra[0] < 0.5 ||
				    _vertices[_indices[j+2]].extra[0] < 0.5)
				{
					variagated = true;
					break;
				}
			}
		}

		if (variagated)
		{
			_vertices[i].color[0] = 0;
			_vertices[i].color[1] = 0;
			_vertices[i].color[2] = 0;
		}
		else
		{
			if (_vertices[i].color[0] < 1e-6 &&
			    _vertices[i].color[1] < 1e-6 &&
			    _vertices[i].color[2] < 1e-6)
			{
				continue;
			}

			_vertices[i].color[0] = red;
			_vertices[i].color[1] = green;
			_vertices[i].color[2] = blue;
		}
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].extra[0] = 0;
	}
}

void Structure::removeColouring()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (_vertices[i].color[0] < 1e-6 &&
		    _vertices[i].color[1] < 1e-6 &&
		    _vertices[i].color[2] < 1e-6)
		{
			continue;
		}

		_vertices[i].color[0] = 0.8;
		_vertices[i].color[1] = 0.8;
		_vertices[i].color[2] = 0.8;
	}
}

double Structure::recalculateDampening(vec3 loc)
{
	if (!pointInside(loc))
	{
		return 1.0;
	}

	vec3 nearest = nearestVertex(loc, true);
	vec3 diff = vec3_subtract_vec3(loc, nearest);
	double d = vec3_length(diff);

	if (d < 4)
	{
		return 1.0;
	}

	d -= 4;
	d *= 2;
	double val = 1.0 * exp(-(d*d));

	return val;
}

vec3 Structure::nearestNormal(vec3 pos, bool useMesh)
{
	Vertex *vClose = nearestVertexPtr(pos, useMesh);
	vec3 v = vec_from_pos(vClose->normal);
	return v;
}

vec3 Structure::lookupVertexPtr(vec3 pos, bool wantNormal)
{
	if (mesh() == NULL)
	{
		if (wantNormal)
		{
			return nearestNormal(pos, true);
		}
		return nearestVertex(pos, true);
	}

	if (_vertexPtrs.size() == 0)
	{
		generateLookupGrid();
	}
  
	if (!checkLocation(pos))
	{
		if (wantNormal)
		{
			return nearestNormal(pos, true);
		}
		return nearestVertex(pos, true);
	}

	double closest = FLT_MAX;
	vec3 vClose = empty_vec3();
	long n = findIndex(pos);
	
	if (_vertexPtrs[n].size() == 0)
	{
		if (wantNormal)
		{
			return nearestNormal(pos, true);
		}
		return nearestVertex(pos, true);
	}
	
	int best_i = -1;
	for (size_t i = 0; i < _vertexPtrs[n].size(); i++)
	{
		vec3 near = _vertexPtrs[n][i];
		vec3 diff = vec3_subtract_vec3(pos, near);

		if (abs(diff.x) > closest || abs(diff.y) > closest 
		    || abs(diff.z) > closest)
		{
			continue;
		}

		double length = vec3_sqlength(diff);
		
		if (length < closest)
		{
			closest = length;
			vClose = near;
			best_i = i;
		}
	}

	if (wantNormal)
	{
		if (best_i >= 0)
		{
			return _normalPtrs[n][best_i];
		}
		else
		{
			return nearestNormal(pos, true);
		}
	}
	
	if (vec3_length(vClose) < 1e-6)
	{
		return nearestVertex(pos, true);
	}
	
	return vClose;
}

void Structure::generateLookupGrid()
{
	boundaries(&_min, &_max);
	_min.x -= 2; _min.y -= 2; _min.z -= 2; 
	_max.x += 2; _max.y += 2; _max.z += 2; 
	vec3 size = vec3_subtract_vec3(_max, _min);
	_nx = ceil(size.x);
	_ny = ceil(size.y);
	_nz = ceil(size.z);
	
	std::cout << "Making grid of size " << _nx << " " << _ny << " " 
	<< _nz << "." << std::endl;

	_dampening.resize(_nz * _ny * _nx);
	
	if (mesh() != NULL)
	{
		_vertexPtrs.resize(_nz * _ny * _nx);
		_normalPtrs.resize(_nz * _ny * _nx);
	}
	
	int max_stages = 100;
	size_t per_stage = _dampening.size() / max_stages;
	size_t stages = 0;

	std::cout << "Generating quick lookup table: " << std::flush;
	for (size_t i = 0; i < _dampening.size(); i++)
	{
		vec3 loc = findLocation(i);
		_dampening[i] = recalculateDampening(loc);

		double close = 5.0;
		vec3 normtot = empty_vec3();
		double count = 0;
		for (size_t j = 0; mesh() != NULL && j < mesh()->vertexCount(); j++)
		{
			Helen3D::Vertex v = mesh()->vertex(j);
			vec3 diff = make_vec3(loc.x - v.pos[0],
			                      loc.y - v.pos[1],
			                      loc.z - v.pos[2]);

			if (abs(diff.x) > close || abs(diff.y) > close 
			    || abs(diff.z) > close)
			{
				continue;
			}

			double length = vec3_sqlength(diff);

			if (length < close)
			{
				vec3 nearest = vec_from_pos(v.pos);
				_vertexPtrs[i].push_back(nearest);

				vec3 normal = vec_from_pos(v.normal);
				_normalPtrs[i].push_back(normal);
			}
		}
		
		if (i > stages + per_stage)
		{
			stages += per_stage;
			std::cout << "+" << std::flush;
		}
	}
	
	std::cout << " ... Done." << std::endl;
}

vec3 Structure::findLocation(long ele)
{
	long x = ele % _nx;
	ele -= x;
	ele /= _nx;

	long y = ele % _ny;
	ele -= y;
	ele /= _ny;

	long z = ele;

	vec3 loc = make_vec3(x + 0.5, y + 0.5, z + 0.5);
	vec3_add_to_vec3(&loc, _min);
	return loc;
}

bool Structure::checkLocation(vec3 loc)
{
	if (loc.x > _max.x - 2 || loc.y > _max.y - 2 || loc.z > _max.z - 2)
	{
		return false;
	}

	if (loc.x < _min.x + 2 || loc.y < _min.y + 2 || loc.z < _min.z + 2)
	{
		return false;
	}
	
	return true;
}

long Structure::findIndex(vec3 loc)
{
	vec3_subtract_from_vec3(&loc, _min);
	loc.x = floor(loc.x);
	loc.y = floor(loc.y);
	loc.z = floor(loc.z);

	long ele = loc.x + _nx * loc.y + (_nx * _ny) * loc.z;
	return ele;
}

double Structure::getDampening(vec3 loc)
{
	if (_dampening.size() == 0)
	{
		generateLookupGrid();
	}
	
	bool check = checkLocation(loc);
	
	if (!check)
	{
		return 1;
	}

	return cubic_interpolate(loc);
	long idx = findIndex(loc);
	return _dampening[idx];
}

void Structure::triangulate()
{
	if (_triangulation >= 2)
	{
		return;
	}
	
	SlipObject::triangulate();
	if (_triangulation == 0)
	{
//		calculateNormalsAndCheck();
	}

	_triangulation++;
}

/* 11-point interpolation - attempted transcription from Dave's function
 * from GAP */
double Structure::cubic_interpolate(vec3 vox000)
{
	vec3_subtract_from_vec3(&vox000, _min);

	/* Pick out just the real components - this is faster
	 * than modf */
	vec3 uvw = make_vec3(vox000.x - floor(vox000.x),
	                     vox000.y - floor(vox000.y),
	                     vox000.z - floor(vox000.z));

	/* Extra refers to the additional index to be fished
	 * for 11-point interpolation. We already get 0 and 1. */
	int extra[3] = {-1, -1, -1};
	int central[3] = {0, 0, 0};
	int next[3] = {1, 1, 1};
	
	/* If uvw components are greater than 0.5, then flip them 
	 * make the extra index one ahead and reverse the order */
	for (int i = 0; i < 3; i++)
	{
		if (*(&uvw.x + i) > 0.5)
		{
			extra[i] = 2;
			central[i] = 1;
			next[i] = 0;
			*(&uvw.x + i) = 1 - *(&uvw.x + i);
		}
	}

	int vox000x = vox000.x + central[0];
	int vox000y = vox000.y + central[1];
	int vox000z = vox000.z + central[2];
	int vox000xm = vox000.x + next[0];
	int vox000ym = vox000.y + next[1];
	int vox000zm = vox000.z + next[2];
	int vox000xn = vox000.x + extra[0];
	int vox000yn = vox000.y + extra[1];
	int vox000zn = vox000.z + extra[2];

	vox000y  *= _nx;
	vox000ym *= _nx;
	vox000yn *= _nx;
	vox000z  *= _nx * _ny;
	vox000zm *= _nx * _ny;
	vox000zn *= _nx * _ny;

	long idx000 = vox000x + vox000y + vox000z;
	long idx100 = vox000xm + vox000y + vox000z;
	long idx010 = vox000x + vox000ym + vox000z;
	long idx110 = vox000xm + vox000ym + vox000z;
	long idx001 = vox000x + vox000y + vox000zm;
	long idx101 = vox000xm + vox000y + vox000zm;
	long idx011 = vox000x + vox000ym + vox000zm;
	long idx111 = vox000xm + vox000ym + vox000zm;
	
	long idxn00 = vox000xn + vox000y + vox000z;
	long idx0n0 = vox000x + vox000yn + vox000z;
	long idx00n = vox000x + vox000y + vox000zn;
	
	double u = uvw.x;
	double v = uvw.y;
	double w = uvw.z;
	
	double p000 = _dampening[idx000];
	double p001 = _dampening[idx001];
	double p010 = _dampening[idx010];
	double p011 = _dampening[idx011];
	double p100 = _dampening[idx100];
	double p101 = _dampening[idx101];
	double p110 = _dampening[idx110];
	double p111 = _dampening[idx111];
	
	double a = p100 - p000;
	double b = p010 - p000;
	double c = p110 - p010;
	double d = p101 - p001;
	
	double pn00 = _dampening[idxn00];
	double p0n0 = _dampening[idx0n0];
	double p00n = _dampening[idx00n];

	double p8value = p000+u*(a+w*(-a+d)+v*((c-a)+w*( a-c-d-p011+p111)))
	+ v*(b+w*(-p001+p011-b))+w*(-p000+p001);
	
	double mod = (p000 - 0.5 * p100 - 0.5 * pn00) * (u - u * u);
	mod += (p000 - 0.5 * p010 - 0.5 * p0n0) * (v - v * v);
	mod += (p000 - 0.5 * p001 - 0.5 * p00n) * (w - w * w);
	
	double p11value = p8value + 0.4 * mod;

	return p11value;
}

void Structure::highlightResidues(std::string result)
{
	std::vector<std::string> segments = split(result, ',');
	std::cout << "Highlighting " << result << std::endl;

	for (int i = 0; i < segments.size(); i++)
	{
		std::string seg = segments[i];
		trim(seg);
		
		std::vector<std::string> bounds = split(seg, '-');
		
		if (bounds.size() == 1)
		{
			markExtraResidue(atoi(bounds[0].c_str()));
		}
		else if (bounds.size() == 2)
		{
			int bound0 = atoi(bounds[0].c_str());
			int bound1 = atoi(bounds[1].c_str());
			
			int min = std::min(bound0, bound1);
			int max = std::max(bound0, bound1);
			
			for (int i = min; i < max; i++)
			{
				markExtraResidue(i);
			}
		}
		else
		{
			std::cout << "Ignoring: " << seg << std::endl;
		}
	}
	
	convertExtraToColour(0.5, 0.5, 0.5);
}

void Structure::heatToVertex(Helen3D::Vertex &v, double heat)
{
	if (heat < 0) heat = 0;
	vec3 colour_start, colour_aim;
	if (heat >= -1 && heat < 0.5)
	{
		colour_start = make_vec3(0.4, 0.4, 0.4); // grey
		colour_aim = make_vec3(0.55, 0.45, 0.29); // straw
	}
	else if (heat >= 0.5 && heat < 1)
	{
		colour_start = make_vec3(0.55, 0.45, 0.29); // straw
		colour_aim = make_vec3(0.39, 0.46, 0.68); // blue
	}
	else if (heat >= 1 && heat < 2)
	{
		colour_start = make_vec3(0.39, 0.46, 0.68); // blue
		colour_aim = make_vec3(0.68, 0.16, 0.08); // cherry red
	}
	else if (heat >= 2 && heat < 3)
	{
		colour_start = make_vec3(0.68, 0.16, 0.08); // cherry red
		colour_aim = make_vec3(0.92, 0.55, 0.17); // orange
	}
	else if (heat >= 3)
	{
		colour_start = make_vec3(0.92, 0.55, 0.17); // orange
		colour_aim = make_vec3(0.89, 0.89, 0.16); // yellow
	}

	double mult = heat - 1;
	if (mult < 0) mult = 0;
	mult *= 3;
	heat = fmod(heat, 1);
	colour_aim -= colour_start;
	vec3_mult(&colour_aim, heat);
	colour_start += colour_aim;
	pos_from_vec(v.color, colour_start);
	vec3_mult(&colour_start, mult);
	pos_from_vec(v.extra, colour_start);
}

void Structure::heatMap()
{
	double sum = 0;
	double sumsq = 0;
	double count = 0;

	for (size_t i = 0; i < vertexCount(); i++)
	{
		double val = _vertices[i].tex[0];
		sum += val;
		sumsq += val * val;
		count++;
	}
	
	double stdev = sqrt(sumsq / count - (sum / count) * (sum / count));
	std::cout << "Standard deviation: " << stdev << std::endl;

	for (size_t i = 0; i < vertexCount(); i++)
	{
		double val = _vertices[i].tex[0] / stdev;
		heatToVertex(_vertices[i], val);
		_vertices[i].tex[0] = 0;
	}
}
