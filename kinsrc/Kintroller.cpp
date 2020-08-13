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

#include "Kintroller.h"
#include "Curve.h"
#include "KineticView.h"
#include "CurveView.h"
#include <FileReader.h>
#include <QThread>
#include <fstream>

Kintroller *Kintroller::_kintroller = NULL;

Kintroller::Kintroller()
{
	_dryRun = false;
	_tree = NULL;
	_view = NULL;
	_cView = NULL;
	_kintroller = this;
}

void Kintroller::setCommandLineArgs(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string str = argv[i];
		
		if (str == "--dry-run")
		{
			_dryRun = true;
			continue;
		}

		_args.push_back(str);
	}
}

void Kintroller::setView(KineticView *view)
{
	_view = view;
	_cView = view->curveView();
	_tree = view->tree();
	
	_resultFile = findNextFilename("kineticresults.csv");
	collectResult("first,second,val");
}

bool Kintroller::getNicknameArguments(std::string line, std::string *nn,
                                      std::vector<std::string> *args)
{
	size_t equal = line.find("=");
	if (equal == std::string::npos) { return false; }

	*nn = line.substr(0, equal);
	trim(*nn);
	if (nn->length() == 0) { return false; }
	if (line.size() <= equal + 1) { return false; }
	equal++;
	
	size_t openbracket = line.find("{", equal);
	if (openbracket == std::string::npos) { return false; }
	if (line.size() <= openbracket + 1) { return false; }

	size_t closebracket = line.find("}", openbracket);
	openbracket++;
	if (closebracket == std::string::npos) { return false; }
	size_t brcklength = closebracket - openbracket;

	std::string contents = line.substr(openbracket, brcklength);
	std::vector<std::string> arguments = split(contents, ',');
	
	for (size_t i = 0; i < arguments.size(); i++)
	{
		trim(arguments[i]);
	}

	*args = arguments;
	return true;
}

void applyRegionToCurve(Curve *c, int which, Region &r)
{
	c->setRegion(which, r.min, r.max);
}

bool Kintroller::processCurve(Curve *c)
{
	_tree->setCurrentItem(c);

	if (!_dryRun)
	{
		c->calculate();
	}

	return true;
}

Curve *Kintroller::makeCurve(std::string nickname, std::string filename,
                             std::string region1, std::string region2)
{
	Curve *c = new Curve(_tree);
	c->setNickname(nickname);
	c->setFilename(filename);
	c->loadData();
	c->setCurveView(_cView);
	_tree->addTopLevelItem(c);
	applyRegionToCurve(c, 0, _regions[region1]);
	applyRegionToCurve(c, 1, _regions[region2]);
	_curves[nickname] = c;
	return c;
}

bool Kintroller::makeCompetition(std::string line)
{
	std::vector<std::string> arguments;
	std::string nickname;

	bool success = getNicknameArguments(line, &nickname, &arguments);
	
	if (arguments.size() < 6 || !success)
	{
		return false;
	}

	std::string filename = arguments[0];
	std::string region1 = arguments[1];
	std::string region2 = arguments[2];
	std::string onoff1 = arguments[3];
	std::string onoff2 = arguments[4];
	std::string buffer = arguments[5];
	
	if (_regions.count(region1) == 0)
	{
		std::cout << "What is " << region1 << "?" << std::endl;
		return false;
	}
	else if (_regions.count(region2) == 0)
	{
		std::cout << "What is " << region2 << "?" << std::endl;
		return false;
	}
	else if (_curves.count(onoff1) == 0)
	{
		std::cout << "What is " << onoff1 << "?" << std::endl;
		return false;
	}
	else if (_curves.count(onoff2) == 0)
	{
		std::cout << "What is " << onoff2 << "?" << std::endl;
		return false;
	}
	else if (_curves.count(buffer) == 0)
	{
		std::cout << "What is " << buffer << "?" << std::endl;
		return false;
	}
	else if (_curves[onoff1]->type() != CurveOnOff)
	{
		std::cout << buffer << " is not a on/off curve?" << std::endl;
		return false;
	}
	else if (_curves[onoff2]->type() != CurveOnOff)
	{
		std::cout << buffer << " is not a on/off curve?" << std::endl;
		return false;
	}

	Curve *c = makeCurve(nickname, filename, region1, region2);
	c->setType(CurveCompetition);

	Curve *b = _curves[buffer];
	c->setBuffer(b);
	Curve *ab1 = _curves[onoff1];
	c->setFirstAb(ab1);
	Curve *ab2 = _curves[onoff2];
	c->setSecondAb(ab2);

	return processCurve(c);
}

bool Kintroller::makeOnOff(std::string line)
{
	std::vector<std::string> arguments;
	std::string nickname;

	bool success = getNicknameArguments(line, &nickname, &arguments);
	
	if (arguments.size() < 4 || !success)
	{
		return false;
	}

	std::string filename = arguments[0];
	std::string region1 = arguments[1];
	std::string region2 = arguments[2];
	std::string buffer = arguments[3];
	
	if (_regions.count(region1) == 0)
	{
		std::cout << "What is " << region1 << "?" << std::endl;
		return false;
	}
	else if (_regions.count(region2) == 0)
	{
		std::cout << "What is " << region2 << "?" << std::endl;
		return false;
	}
	else if (_curves.count(buffer) == 0)
	{
		std::cout << "What is " << buffer << "?" << std::endl;
		return false;
	}
	else if (_curves[buffer]->type() != CurveBuffer)
	{
		std::cout << buffer << " is not a buffer curve?" << std::endl;
		return false;
	}

	Curve *c = makeCurve(nickname, filename, region1, region2);
	c->setType(CurveOnOff);

	Curve *b = _curves[buffer];
	c->setBuffer(b);

	return processCurve(c);
}

bool Kintroller::makeBuffer(std::string line)
{
	std::vector<std::string> arguments;
	std::string nickname;

	bool success = getNicknameArguments(line, &nickname, &arguments);
	
	if (arguments.size() < 3 || !success)
	{
		return false;
	}

	std::string filename = arguments[0];
	std::string region1 = arguments[1];
	std::string region2 = arguments[2];
	
	if (_regions.count(region1) == 0)
	{
		std::cout << "What is " << region1 << "?" << std::endl;
		return false;
	}
	else if (_regions.count(region2) == 0)
	{
		std::cout << "What is " << region2 << "?" << std::endl;
		return false;
	}

	Curve *c = makeCurve(nickname, filename, region1, region2);
	c->setType(CurveBuffer);

	return processCurve(c);
}

bool Kintroller::makeRegion(std::string line)
{
	std::vector<std::string> arguments;
	std::string nickname;

	bool success = getNicknameArguments(line, &nickname, &arguments);
	
	if (arguments.size() < 2 || !success)
	{
		return false;
	}

	std::string startstr = arguments[0];
	std::string endstr = arguments[1];
	
	Region r;
	r.min = atof(startstr.c_str());
	r.max = atof(endstr.c_str());
	_regions[nickname] = r;

	return true;
}

bool Kintroller::processLine(std::string line)
{
	size_t space = line.find(" ");
	
	if (space == std::string::npos)
	{
		return false;
	}

	std::string command = line.substr(0, space);

	if (line.size() <= space + 1)
	{
		return false;
	}
	
	if (command == "buffer")
	{
		std::string rest = line.substr(space + 1, std::string::npos);
		return makeBuffer(rest);
	}
	else if (command == "region")
	{
		std::string rest = line.substr(space + 1, std::string::npos);
		return makeRegion(rest);
	}
	else if (command == "onoff")
	{
		std::string rest = line.substr(space + 1, std::string::npos);
		return makeOnOff(rest);
	}
	else if (command == "competition")
	{
		std::string rest = line.substr(space + 1, std::string::npos);
		return makeCompetition(rest);
	}
	
	return false;
}

void Kintroller::run()
{
	if (_args.size() <= 0)
	{
		return;
	}

	std::string fn = get_file_contents(_args[0]);
	std::vector<std::string> lines = split(fn, '\n');
	
	for (size_t i = 0; i < lines.size(); i++)
	{
		std::string line = lines[i];
		
		if (line.length() == 0)
		{
			continue;
		}
		
		if (line[0] == '#')
		{
			std::cout << line << std::endl;
			continue;
		}

		bool success = processLine(line);
		
		if (!success)
		{
			std::cout << "Error on line " << i << std::endl;
			std::cout << "\t" << line << std::endl;
		}
	}
}

void Kintroller::collectResult(std::string result)
{
	std::ofstream resf;
	resf.open(_resultFile, std::ios_base::app);

	resf << result << std::endl;
	resf.close();
}
