#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QThread>
#include <QAbstractNativeEventFilter>
#include <QKeyEvent>
#include "mainwindow.h"
#include "utils.h"
#include <sysfn/os.h>
#include <sysfn/time_query.h>
#include <time.h>

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
//# define NOMINMAX
#	include <windows.h>
#endif

namespace sys {
	hptimer_t g_Start = 0, g_Freq = 1000000;
}

FILE * g_LogRedirect = 0;

struct Application : QApplication, public QAbstractNativeEventFilter
{
	MainWindow * m_main_window;

	Application (int & argc, char * argv[])
		: QApplication(argc, argv)
		, m_main_window(0)
	{}

	~Application () { }

	void setMainWindow (MainWindow * mw)
	{
		installNativeEventFilter(this);
		m_main_window = mw;
	}

#ifdef WIN32
	virtual bool nativeEventFilter(QByteArray const & eventType, void * message, long * result)
	{
		DWORD const hotkey = VK_SCROLL;
		MSG * msg = static_cast<MSG *>(message);
		if (msg->message == WM_HOTKEY)
		{
			qDebug("wineventfilter hotkey");
			//if (GetKeyState(hotkey))
				if (m_main_window)
					m_main_window->onHotkeyShowOrHide();
			return true;
		}
		return false;
	}
#endif

	virtual bool notify (QObject * receiver, QEvent * e)
	{
		try
		{
			return QApplication::notify(receiver, e);
		}
		catch (std::exception const & ex)
		{
			qFatal("Error %s sending event %s to object %s (%s)",
				ex.what(), typeid(*e).name(), qPrintable(receiver->objectName()),
				typeid(*receiver).name());
			return true;
		}
		catch (...)
		{
			qFatal("Error <unknown> sending event %s to object %s (%s)",
				typeid(*e).name(), qPrintable(receiver->objectName()),
				typeid(*receiver).name());
			return true;
		}
		return false;
	}
};

void usage ()
{
	printf("\nDbgToolkit's trace server, Copyright (C) 2011-2014 Mojmir Svoboda\n");
	printf("https://github.com/mojmir-svoboda/DbgToolkit\n\n");
	printf("Available options:\n");
	printf("\t\t-q\t\tquit immeadiately if another instance running\n");
	printf("\t\t-n\t\tno visible window at start (can be activated by ScrollLock hotkey)\n");
	printf("\t\t-d\t\tdump mode (csv by default)\n");
}

void qDebugHandler (QtMsgType type, QMessageLogContext const & ctx, QString const & msg)
{
	time_t timer;
	tm * tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	char t[48];
	strftime(t, 25, "%Y:%m:%d %H:%M:%S", tm_info);

	//@TODO: dump context info
	switch (type)
	{
		case QtDebugMsg:
			fprintf(g_LogRedirect, "%s|I|%x|%s\n", t, sys::get_tid(), msg.toLatin1().data());
			break;
		case QtWarningMsg:
			fprintf(g_LogRedirect, "%s|W|%x|%s\n", t, sys::get_tid(), msg.toLatin1().data());
			break;
		case QtCriticalMsg:
			fprintf(g_LogRedirect, "%s|E|%x|%s\n", t, sys::get_tid(), msg.toLatin1().data());
			break;
		case QtFatalMsg:
			fprintf(g_LogRedirect, "%s|F|%x|%s\n", t, sys::get_tid(), msg.toLatin1().data());
			break;
	}
	fflush(g_LogRedirect);
}

int main (int argc, char * argv[])
{
	sys::setTimeStart();

	QString const path = QDir::homePath() + "/" + g_traceServerDirName + "/logs";
	mkPath(path);
	QFileInfo fi(argv[0]);
	QString const log_name = path + "/" + QString("%1.%2").arg(fi.completeBaseName()).arg("log");
	g_LogRedirect = fopen(log_name.toLatin1(), "w");
	bool quit_delay = true;
	bool start_hidden = false;
	bool dump_mode = false;
	int level = -1;
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-')
		{
			//foo.push_back(argv[i]);
			continue;
		}

		// if there's a flag but no argument following, ignore it
		if (argc == i) continue;
		char const * arg = argv[i+1];
		switch (argv[i][1])
		{
			case 'q':
				printf("cmd arg: -q, quit immeadiately\n");
				quit_delay = false;
				break;
			case 'd':
				printf("cmd arg: -d, dump mode\n");
				dump_mode = true;
				break;
			case 'n':
				printf("cmd arg: -n, no visible window\n");
				start_hidden = true;
				break;
			case 'l':
				level = std::atoi(arg);
				printf("cmd arg: -l, start at level %i\n", level);
				break;
			case 'h':
				usage();
				return 0;
			default:
				printf("Invalid option, use -h for Help\n");
				return 0;
		}
	}

	if (g_LogRedirect)
		qInstallMessageHandler(qDebugHandler);

	Application app(argc, argv);

#ifdef WIN32
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::tr("Systray"), QObject::tr("I couldn't detect any system tray on this system."));
		return 1;
	}
#endif

	MainWindow w(0, quit_delay, dump_mode, log_name, level);

	if (!start_hidden)
	{
		w.setVisible(true);
		w.show();
	}
	app.setMainWindow(&w);
	if (start_hidden)
		w.onHotkeyShowOrHide();

	bool const retval = app.exec(); // main loop

	qInstallMessageHandler(0);
	if (g_LogRedirect)
		fclose(g_LogRedirect);
	return retval;
}
