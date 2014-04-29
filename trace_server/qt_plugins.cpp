#include <QtPlugin>
#include <QWindow>
#include <QWidget>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>

static QWindow * windowForWidget(const QWidget* widget)
{
	if (QWindow* window = widget->windowHandle()) { return window; }
	if (const QWidget* nativeParent = widget->nativeParentWidget()) { return nativeParent->windowHandle(); } 
	return 0; 
}

HWND getHWNDForWidget (QWidget const * widget)
{
	if (QWindow* window = ::windowForWidget(widget))
	{
		if (window->handle()) 
		{
			return static_cast<HWND>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArrayLiteral("handle"), window));
		}
	}
	return 0;
} 



#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#if (defined WIN32) && (defined STATIC)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
	Q_IMPORT_PLUGIN(QICOPlugin);
//	Q_IMPORT_PLUGIN(qsvg); //@TODO: NEZAPOMENOUT ODKOMENTOVAT!
#endif


