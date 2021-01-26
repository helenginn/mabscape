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

#ifndef __mabscape__metadata__
#define __mabscape__metadata__

#include <vector>
#include <map>
#include <QObject>


class QMenu;
class Experiment;
class Bound;

typedef std::map<std::string, std::string> KeyValue;
typedef std::map<Bound *, KeyValue> BoundKeys;

class Metadata : public QObject
{
Q_OBJECT
public:
	Metadata();
	
	void loadBounds(std::vector<Bound *> bound);
	
	void setFilename(std::string filename)
	{
		_filename = filename;
	}
	
	size_t titleCount()
	{
		return _titles.size();
	}
	
	size_t boundCount()
	{
		return _bounds.size();
	}
	
	Bound *bound(int i)
	{
		return _bounds[i];
	}
	
	void makeMenu(QMenu *m, Experiment *e);
	void colourBy(std::string title);

	void load();
private:
	std::string _filename;

	std::vector<std::string> _titles;
	std::map<std::string, Bound *> _names;
	std::vector<Bound *> _bounds;
	BoundKeys _keys;
};

#endif
