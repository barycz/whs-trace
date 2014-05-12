#pragma once
#include <tableview.h>
class Connection;

namespace logs {

	class LogWidget; struct LogConfig;

	class LogTableView : public TableView
	{
	public:
		LogTableView (Connection * conn, LogWidget & logwidget, LogConfig & config);
		virtual ~LogTableView ();

		LogConfig & config () { return m_config; }
		LogConfig const & config () const { return m_config; }

		virtual void scrollTo (QModelIndex const & index, ScrollHint hint = EnsureVisible);
		virtual void wheelEvent (QWheelEvent * event);
		virtual void keyPressEvent (QKeyEvent * event);
		virtual QModelIndex moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

	protected:
		Connection * m_connection;
		LogWidget & m_log_widget;
		LogConfig & m_config;
	};
}

