#pragma once
#include "ui_settingslog.h"

#if defined WIN32
void openFileLineWith (QString const & cmd, QString const & file, QString const & line)
{
	if (cmd.contains("Visual Studio"))
	{
		QString version;
		if (cmd == "Visual Studio 2012")
			version = "12";
		else if (cmd == "Visual Studio 2010")
			version = "10";
		else if (cmd == "Visual Studio 2008")
			version = "8";
		else if (cmd == "Visual Studio 2005")
			version = "5";
		else
			return; // earlier versions are shit
		
		QString const path = QDir::currentPath();
		QString const cmd = QObject::tr("%1\\VisualStudioFileOpenTool.all.exe %2 %3 %4").arg(QDir::toNativeSeparators(path)).arg(version).arg(QDir::toNativeSeparators(file)).arg(line);
		qDebug("open file with cmd: %s", qPrintable(cmd));
		system(qPrintable(cmd));
		return;
	}

	if (cmd.contains("gVim"))
	{
		QString const cmd = QObject::tr("gvim.exe +%1 %2").arg(file).arg(line);
		qDebug("open file with cmd: %s", qPrintable(cmd));
		system(qPrintable(cmd));
	}
}
#endif

//@TODO: linux

