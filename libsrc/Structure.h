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

#ifndef __abmap__Structure__
#define __abmap__Structure__

#include <Frameworks.h>
#include <helen3d/SlipObjFile.h>
#include <helen3d/vec3.h>
#include <libsrc/shared_ptrs.h>

typedef std::map<int, std::vector<Helen3D::Vertex *>> AtomMap;
typedef std::map<Helen3D::Vertex *, int> AtomSingleMap;

class Structure : public SlipObjFile
{
public:
	Structure(std::string filename);
	
	void clearExtra();
	void markExtraAround(vec3 pos, double reach, bool fillout = false);
	void markExtraResidue(int i);
	void convertExtraToColour();
	void removeColouring();
	double getDampening(vec3 loc);
	double recalculateDampening(vec3 loc);
	virtual void triangulate();
	vec3 lookupVertexPtr(vec3 pos);

	void addPDB(std::string filename);
	void turtleShell();
	
	size_t litResidueCount()
	{
		return _resiList.size();
	}
	
	int litResidue(int i)
	{
		return _resiList[i];
	}
	
	std::string residueName(int res)
	{
		return _resNames[res];
	}
	
	vec3 residuePos(int res)
	{
		return _resPos[res];
	}

private:
	long findIndex(vec3 loc);
	void generateLookupGrid();
	bool checkLocation(vec3 loc);
	double cubic_interpolate(vec3 vox000);
	vec3 findLocation(long ele);

	CrystalPtr _crystal;
	int _triangulation;

	std::vector<double> _dampening;
	std::vector<std::vector<vec3> > _vertexPtrs;
	vec3 _min;
	vec3 _max;
	int _nx, _ny, _nz;

	std::map<int, std::string> _resNames;
	std::map<int, vec3> _resPos;
	AtomMap _atomMap;
	AtomSingleMap _singleMap;
	std::vector<int> _resiList;
};

#endif
