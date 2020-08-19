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

#ifndef __abmap__Dialogue__
#define __abmap__Dialogue__

#include <QFileDialog>

inline std::string openDialogue(QWidget *w, QString title, QString pattern)
{
	QFileDialog *f = new QFileDialog(w, title, pattern);
	                                 
	f->setFileMode(QFileDialog::AnyFile);
	f->setOptions(QFileDialog::DontUseNativeDialog);
	f->show();

    QStringList fileNames;

    if (f->exec())
    {
        fileNames = f->selectedFiles();
    }
    
    if (fileNames.size() < 1)
    {
		return "";
    }

	f->deleteLater();
	std::string filename = fileNames[0].toStdString();

	return filename;
}

#endif
