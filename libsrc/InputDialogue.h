//
//  QTinker.h
//  Windexing
//
//  Created by Helen Ginn on 11/12/2016.
//  Copyright (c) 2017 Helen Ginn. All rights reserved.
//

#ifndef __Windexing__QUnitCell__
#define __Windexing__QUnitCell__

#include <stdio.h>

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>

class SurfaceView;

typedef enum
{
	DialogueUndefined,
	DialogueHighlight,
} DialogueType;

class Dialogue : public QMainWindow
{
	Q_OBJECT

public:
	Dialogue(QWidget *parent = 0, std::string windowText = "",
	         std::string labelText = "", std::string defaultText = "",
	         std::string buttonText = "");

	~Dialogue();

	void setWindow(SurfaceView *window)
	{
		_window = window;
	}

	void setTag(DialogueType type)
	{
		_type = type;
	}

private slots:
	void returnClicked();

private:
	QPushButton *bDialogue;
	QLineEdit *tDialogue;
	QLabel *lDialogue;

	SurfaceView *_window;
	DialogueType _type;
};

#endif /* defined(__CaroCode__QTinker__) */
