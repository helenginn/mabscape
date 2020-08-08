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

#include "Result.h"
#include "Structure.h"
#include "Experiment.h"
#include "Bound.h"
#include <fstream>
#include <MtzFFT.h>
#include <MtzFile.h>
#include <FileReader.h>
#include <vector>
#include <QuickAtoms.h>

Result::Result()
{
	_score = 0;
	_exp = NULL;
}

void Result::populateFromString(std::string contents, size_t *firstpos)
{
	if (_exp == NULL)
	{
		std::cout << "Helen..." << std::endl;
		return;
	}

	size_t lastpos = 0;
	lastpos = contents.find("<<<", *firstpos);

	std::string full = contents.substr(*firstpos, lastpos - *firstpos);
	*firstpos = lastpos;

	std::vector<std::string> lines = split(full, '\n');

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::string score = "score: ";
		if (lines[i].find(score) == 0)
		{
			char *pos = &lines[i][score.length() + 1];
			double val = atof(pos);
			setScore(val);
		}
		
		std::vector<std::string> bits = split(lines[i], ',');

		if (bits.size() < 4)
		{
			continue;
		}

		std::string id = bits[0];
		Bound *b = _exp->bound(id);
		if (b == NULL)
		{
			continue;
		}

		vec3 vec;
		vec.x = atof(bits[1].c_str());
		vec.y = atof(bits[2].c_str());
		vec.z = atof(bits[3].c_str());
		
		_map[b] = vec;
	}
}

void Result::applyPositions()
{
	for (size_t i = 0; i < _exp->boundCount(); i++)
	{
		Bound *bi = _exp->bound(i);
		
		if ((bi->isFixed()) || (_map.count(bi) == 0))
		{
			continue;
		}

		vec3 wip = _map[bi];
		bi->setRealPosition(wip);
		bi->updatePositionToReal();
	}
}

void Result::savePositions()
{
	_map.clear();

	for (size_t i = 0; i < _exp->boundCount(); i++)
	{
		vec3 wip = _exp->bound(i)->getWorkingPosition();
		_map[_exp->bound(i)] = wip;
	}

}

vec3 Result::vecForBound(Bound *b)
{
	return _map[b];
}

MtzFFTPtr Result::makeDatasetForCluster4x()
{
	MtzFFTPtr fft = MtzFFTPtr(new MtzFFT(NULL));

	std::string str = "Dataset " + f_to_str(_score, 4);
	MtzFile *file = new MtzFile(str);
	file->setMetadata(str);
	fft->setMtzFile(file);
	fft->setUnitCell(std::vector<double>(6, 0));

	QuickAtoms *atoms = file->getQuickAtoms();
	
	for (size_t i = 0; i < _exp->boundCount(); i++)
	{
		Bound *bi = _exp->bound(i);
		if (_map.count(bi) == 0)
		{
			continue;
		}

		vec3 wip = _map[bi];
		atoms->addSequentialAtom("A", wip);
	}

	return fft;
}

void Result::writeToStream(std::ofstream &file)
{
	file << ">>> begin result" << std::endl;
	file << "score: " << _score << std::endl << std::endl;

	for (size_t i = 0; i < _exp->boundCount(); i++)
	{
		Bound *bi = _exp->bound(i);
		if (_map.count(bi) == 0)
		{
			continue;
		}

		vec3 wip = _map[bi];
		std::string name = bi->name();
		
		file << name << ", " << wip.x << ", " << wip.y
		<< ", " << wip.z << std::endl;
	}
	
	file << "<<< end result" << std::endl;
}

void Result::markVerticesAroundBound(Structure *s, Bound *b)
{
	if (_map.count(b) == 0)
	{
		return;
	}
	
	vec3 wip = _map[b];
	vec3 near = s->nearestVertex(wip);
	s->markExtraAround(near, 4.);
}

