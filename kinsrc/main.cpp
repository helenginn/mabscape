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

#include <iostream>
#include <QApplication>
#include <QThread>
#include "commit.h"
#include "KineticView.h"
#include "Kintroller.h"

int main(int argc, char * argv[])
{
	std::cout << "Abmap Version: " << VAGABOND_VERSION_COMMIT_ID << std::endl;

	QApplication app(argc, argv);
	setlocale(LC_NUMERIC, "C");

	KineticView k(NULL);

	QThread *worker = new QThread();
	
	Kintroller *con = new Kintroller();
	con->setCommandLineArgs(argc, argv);

	k.show();
	k.startController(con, worker);

	int status = app.exec();

	return status;
}
