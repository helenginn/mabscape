//
//  main.cpp
//  abmap
//
//  Created by Helen Ginn on 11/07/2017.
//  Copyright (c) 2017 Strubi. All rights reserved.
//

#include <execinfo.h>
#include "Controller.h"
#include <signal.h>
#include <iostream>
#include <Options.h>
#include <QApplication>
#include <QThread>
#include <QObject>
#include "SurfaceView.h"
#include "Experiment.h"
#include "commit.h"

int main(int argc, char * argv[])
{
	std::cout << "Hi!" << std::endl;

	QApplication app(argc, argv);
	setlocale(LC_NUMERIC, "C");

	std::cout << "Abmap Version: " << VAGABOND_VERSION_COMMIT_ID << std::endl;
	const char hang[] = "--hang";
	OptionsPtr options = OptionsPtr(new Options(1, (const char **)&hang));
	Options::setRuntimeOptions(options);

	SurfaceView s;

	QThread *worker = new QThread();
	
	Controller *con = new Controller();
	con->setCommandLineArgs(argc, argv);

	s.show();
	s.startController(worker, con);

	int status = app.exec();

	return status;
	return 0;
}
