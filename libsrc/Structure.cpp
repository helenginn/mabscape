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
#include <iostream>
#include <PDBReader.h>
#include <Monomer.h>
#include <Crystal.h>
#include <Atom.h>

Structure::Structure(std::string filename) : SlipObjFile(filename)
{
	resize(1);
	setName("Structure");
	recolour(0, 0.5, 0.5);
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
		
		if (atom)
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
	std::vector<Vertex *> vs = _atomMap[resi];

	for (size_t i = 0; i < vs.size(); i++)
	{
		if (vs[i]->color[0] < 1e-6 &&
		    vs[i]->color[1] < 1e-6 &&
		    vs[i]->color[2] < 1e-6)
		{
			continue;
		}

		vs[i]->color[0] = 0.4;
		vs[i]->color[1] = 0.4;
		vs[i]->color[2] = 1.0;
	}
}

void Structure::convertExtraToColour()
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

			_vertices[i].color[0] = 0.8;
			_vertices[i].color[1] = 0.8;
			_vertices[i].color[2] = 0.8;
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
