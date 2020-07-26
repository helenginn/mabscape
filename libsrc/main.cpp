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

int main(int argc, char * argv[])
{
	std::cout << "Hi!" << std::endl;

	QApplication app(argc, argv);
	setlocale(LC_NUMERIC, "C");

	SurfaceView s;
	s.show();

	int status = app.exec();

	return status;
	return 0;
}
