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

#include "Genes.h"
#include "Antibody.h"
#include <fstream>
#include <FileReader.h>

Genes::Genes()
{

}

void Genes::compare(std::vector<Antibody *> abs)
{
	std::ofstream heavyfile, lightfile;
	heavyfile.open("heavy_data.csv");
	lightfile.open("light_data.csv");

	for (size_t i = 1; i < abs.size(); i++)
	{
		for (size_t j = 0; j < i; j++)
		{
			double hval = abs[i]->compareWithAntibody(abs[j], true);
			double lval = abs[i]->compareWithAntibody(abs[j], false);
			heavyfile << abs[i]->name() << "," << abs[j]->name()
			<< "," << hval << std::endl;

			lightfile << abs[i]->name() << "," << abs[j]->name()
			<< "," << lval << std::endl;
		}
	}
	
	heavyfile.close();
	lightfile.close();
}

void Genes::loadSequences(std::string filename)
{
	std::cout << "Should load aa sequences from " << filename << std::endl;

	std::string contents;
	
	try
	{
		contents = get_file_contents(filename);
	}
	catch (int e)
	{
		std::cout << "Could not load filename " << filename << std::endl;
		return;
	}
	
	std::vector<std::string> lines = split(contents, '\n');
	std::vector<Antibody *> abs;

	for (size_t i = 1; i < lines.size(); i++)
	{
		std::vector<std::string> components = split(lines[i], ',');
		
		if (components.size() != 3)
		{
			if (lines[i].length() > 0)
			{
				std::cout << "Wrong number of terms, skipping: " << std::endl;
				std::cout << lines[i] << std::endl;
			}

			continue;
		}

		std::string id = components[0];
		std::string haa = components[1];
		std::string laa = components[2];
		
		Antibody *ab = new Antibody(id, haa, laa);
		abs.push_back(ab);
	}
	
	std::cout << abs.size() << " antibodies loaded" << std::endl;
	std::cout << std::endl;
	
	compare(abs);
}

