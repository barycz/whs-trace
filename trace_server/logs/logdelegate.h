#pragma once
#include <QStyledItemDelegate>
#include "appdata.h"
#include "logwidget.h"

namespace logs {

	class LogWidget;

	class LogDelegate : public QStyledItemDelegate
	{
		LogWidget const & m_log_widget;
		AppData const & m_app_data;
	public:
		LogDelegate (LogWidget & lw, AppData const & ad, QObject * parent = 0);
		virtual ~LogDelegate() { }

		void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void paintCustom (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index, QString const & separator, QString const & out_separator, int level = 1) const;
		void paintContext (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
		void paintTime (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
		void paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const;
	};

}

