//
//  main.cpp
//  abmap
//
//  Created by Helen Ginn on 11/07/2017.
//  Copyright (c) 2017 Strubi. All rights reserved.
//

#include <execinfo.h>
#include <signal.h>
#include <iostream>
#include <QApplication>
#include "SurfaceView.h"
#include "Experiment.h"

int main(int argc, char * argv[])
{
	std::cout << "Hi!" << std::endl;

	QApplication app(argc, argv);
	setlocale(LC_NUMERIC, "C");

	SurfaceView s;
	Experiment *e = s.getExperiment();
	
	std::cout << "Argc: " << argc << std::endl;
	if (argc > 1)
	{
		std::string file = argv[1];
		std::cout << "Loading" << std::endl;
		e->loadStructure(file);
	}

	if (argc > 2)
	{
		std::cout << "Setting bound object" << std::endl;
		e->setBoundObj(argv[2]);
	}

	s.show();

	int status = app.exec();

	return status;
	return 0;
}
