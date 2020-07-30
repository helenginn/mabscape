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

#include "SlipObjFile.h"
#include <FileReader.h>

SlipObjFile::SlipObjFile(std::string filename) : SlipObject()
{
	_filename = filename;
	readInput();
}

void SlipObjFile::readInput()
{
	std::vector<vec3> normals;
	std::string contents = get_file_contents(_filename);
	std::vector<std::string> lines = split(contents, '\n');

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> bits = split(lines[i], ' ');

		if (bits.size() < 1)
		{
			continue;
		}

		if (bits[0] == "v")
		{
			if (bits.size() < 4)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				continue;
			}

			float v1 = atof(bits[1].c_str());
			float v2 = -atof(bits[2].c_str());
			float v3 = atof(bits[3].c_str());

			addVertex(v1, v2, v3);
		}

		if (bits[0] == "vn")
		{
			if (bits.size() < 4)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				continue;
			}

			vec3 norm;
			norm.x = atof(bits[1].c_str());
			norm.y = atof(bits[2].c_str());
			norm.z = atof(bits[3].c_str());

			normals.push_back(norm);
		}

		if (bits[0] == "f")
		{
			if (bits.size() != 4)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				std::cout << "\t(can only deal with 3 numbers)" << std::endl;
				continue;
			}

			for (size_t j = 1; j < bits.size(); j++)
			{
				std::vector<std::string> components = split(bits[j], '/');
				GLuint index = 0;
				
				if (components.size() > 0)
				{
					index = atoi(components[0].c_str()) - 1;
					addIndex(index);
				}
				
				if (components.size() > 2)
				{
					GLuint nindex = atoi(components[2].c_str()) - 1;
					if (normals.size() <= nindex)
					{
						std::cout << "Warning: out of bounds normal: " <<
						std::endl;
						std::cout << "\t" << lines[i] << std::endl;
					}

					vec3 norm = normals[nindex];
					_vertices[index].normal[0] = norm.x;
					_vertices[index].normal[1] = norm.y;
					_vertices[index].normal[2] = norm.z;
				}
				
			}
		}
	}
}

