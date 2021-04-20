// mabscape
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

#include <QMenu>
#include <iostream>
#include <algorithm>
#include <hcsrc/FileReader.h>

#include "Experiment.h"
#include "ColourOptions.h"
#include "SurfaceView.h"
#include "Metadata.h"
#include "Bound.h"

Metadata::Metadata()
{

}

void Metadata::load()
{
	if (!file_exists(_filename))
	{
		std::cout << "File does not exist" << std::endl;
		return;
	}

	std::string contents = get_file_contents(_filename);
	std::vector<std::string> lines = split(contents, '\n');
	
	if (lines.size() == 0)
	{
		std::cout << "File empty" << std::endl;
		return;
	}
	
	std::string header = lines[0];
	std::vector<std::string> titles = split(header, ',');
	
	if (titles.size() == 0)
	{
		std::cout << "Lines are empty" << std::endl;
		return;
	}
	
	std::cout << "We assume " << titles[0] << " is the "\
	"sequence identifier. If this is not the case, "\
	"fix and reload" << std::endl;
	
	size_t skip = 0;
	size_t count = 0;
	
	for (size_t i = 1; i < lines.size(); i++)
	{
		std::vector<std::string> components = split(lines[i], ',');
		
		if (components.size() != titles.size() || components.size() == 0)
		{
			std::cout << "Skipping line " << i << " - incorrect number "\
			" of entries." << std::endl;
			std::cout << "\t" << lines[i] << std::endl;
			continue;
		}
		
		KeyValue kv;
		
		if (_names.count(components[0]) == 0)
		{
			skip++;
			continue;
		}

		trim(components[0]);
		Bound *which = _names[components[0]];
		
		if (which == NULL)
		{
			continue;
		}
		else
		{
			kv = _keys[which];
		}

		for (size_t j = 1; j < components.size(); j++)
		{
			trim(components[j]);
			kv[titles[j]] = components[j];
		}
		
		_keys[which] = kv;

		count++;
	}


	for (size_t i = 1; i < titles.size(); i++)
	{
		bool found = std::find(_titles.begin(), _titles.end(), 
		                       titles[i]) != _titles.end();

		if (!found)
		{
			_titles.push_back(titles[i]);
		}
	}

	std::cout << "Skipped " << skip << " antibodies not in memory." << std::endl;
	std::cout << "Loaded metadata for " << count << " antibodies." << std::endl;
	std::cout << "Titles are: " << std::endl;
	
	for (size_t i = 0; i < _titles.size(); i++)
	{
		std::cout << "\t" << _titles[i] << std::endl;
	}
}

void Metadata::colourOptions(std::string title)
{
	_title = title;
	ColourOptions *options = new ColourOptions(NULL, this);
	options->show();
}

void Metadata::makeMenu(QMenu *m, Experiment *e)
{
	QList<QAction *> widges = m->findChildren<QAction *>("metadata");
	
	for (size_t i = 0; i < widges.size(); i++)
	{
		m->removeAction(widges[i]);
	}

	for (size_t i = 0; i < _titles.size(); i++)
	{
		QString qTitle = QString::fromStdString(_titles[i]);

		QAction *act = m->addAction(qTitle);
		act->setObjectName("metadata");
		connect(act, &QAction::triggered, 
		        this, [=]() {colourOptions(_titles[i]);});
	}

}

void Metadata::loadBounds(std::vector<Bound *> bounds)
{
	_bounds = bounds;
	
	for (size_t i = 0; i < boundCount(); i++)
	{
		Bound *b = bound(i);

		_names[b->name()] = b;
	}
}

bool Metadata::hasKey(Bound *b, std::string key)
{
	return _keys.count(b) && _keys[b].count(key);
}

std::string Metadata::valueForKey(Bound *b, std::string key)
{
	return _keys[b][key];
}

void Metadata::colourByScale(bool takeLog, double mean, double stdev)
{
	double sum = 0;
	double sqSum = 0;
	double count = 0;
	
	for (size_t i = 0; i < _bounds.size(); i++)
	{
		Bound *b = bound(i);
		b->setValue(NAN);
		
		KeyValue &kv = _keys[b];
		
		if (kv.count(_title) == 0)
		{
			continue;
		}

		std::string result = kv[_title];
		
		if (result == "")
		{
			continue;
		}

		double value = atof(result.c_str());
		if (takeLog)
		{
			value = log(value) / log(10);
		}

		b->setValue(value);

		sum += value;
		sqSum += value * value;
		count++;

		std::cout << b->name() << " " << value << std::endl;
	}

	double real_mean = sum / count;
	
	if (mean != mean)
	{
		mean = real_mean;
	}

	double real_stdev = sqrt(sqSum / count - real_mean * real_mean);
	
	if (stdev != stdev)
	{
		stdev = real_stdev;
	}
	
	std::cout << mean << " " << stdev << std::endl;

	for (size_t i = 0; i < _bounds.size(); i++)
	{
		Bound *b = bound(i);

		if (b != NULL)
		{
			b->colourByValue(mean, -stdev);
		}
	}
}

bool Metadata::hasTitle(std::string title)
{
	return std::find(_titles.begin(), _titles.end(), title) != _titles.end();
}
