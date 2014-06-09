#include "logdelegate.h"
#include <QObject>
#include <QPainter>
#include <logs/filterproxymodel.h>

namespace logs {

	LogDelegate::LogDelegate (LogWidget & lw, AppData const & ad, QObject * parent)
		: QStyledItemDelegate(parent), m_log_widget(lw), m_app_data(ad)
	{ }

	void LogDelegate::paintContext (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data.getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}

	void LogDelegate::paintTime (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index) const
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			unsigned long long const ref_value = m_log_widget.timeRefValue();
			QVariant value = index.data(Qt::DisplayRole);

			long long const ref_dt = value.toString().toULongLong() - ref_value;

			option4.text.clear();
			/*if (m_log_widget.config().m_dt_enabled)
			{
				if (ref_dt >= 0)
				{
					QAbstractItemModel const * model = m_log_widget.m_tableview->model();
					
					int const col_idx = const_cast<logs::LogWidget &>(m_log_widget).findColumn4Tag(tlv::tag_dt);
					
					if (col_idx >= 0)
					{
						QVariant value;

						// @TODO: nemela by toto proxy delat sama?
						if (FilterProxyModel const * proxy = m_log_widget.logProxy())
						{
							QModelIndex const idx = proxy->index(index.row(), col_idx, QModelIndex());
							QModelIndex const curr = proxy->mapToSource(idx);
							value = model->data(curr);
						}
						else
						{
							QModelIndex const idx = model->index(index.row(), col_idx, QModelIndex());
							value = model->data(idx).toString();
						}

						if (value.isValid())
						{
							option4.text = value.toString();
						}
					}
				}
			}
			else*/
			{
				if (ref_dt >= 0)
					option4.text = tr("%1").arg(ref_dt);
			}

			if (value.isValid())
			{
				QString const & val = option4.text;
				float const t_us = val.toFloat();
				float const t_natural_units = 1000000.0f; // microseconds
				float const t = t_us / t_natural_units / m_log_widget.config().m_time_units;
				option4.text = QString::number(t, 'f', 3);
			}

			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}

	void LogDelegate::paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option4, QModelIndex const & index, QString const & separator, QString const & out_separator, int level) const
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QStringList list = value.toString().split(QRegExp(separator), QString::SkipEmptyParts);
			if (level < list.size())
			{
				QString p;
				for (int i = list.size() - level, ie = list.size(); i < ie; ++i)
				{
					if (i > 0)
						p.append(out_separator);
					p.append(list.at(i));
				}
				option4.text = p;
			}

			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}


	void LogDelegate::paintHilited (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index) const
	{
		if (option.showDecorationSelected && (option.state & QStyle::State_Selected))
		{
			option.font.setBold(true);
		}
		else
		{
			option.font.setBold(false);
		}
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
	}

	void LogDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
	{
		painter->save();
		QStyleOptionViewItemV4 option4 = option;
		initStyleOption(&option4, index);

		std::vector<QString> const & column_aligns = m_log_widget.config().m_columns_align;
		if (index.column() < column_aligns.size())
		{
			E_Align const align = stringToAlign(column_aligns[index.column()].at(0).toLatin1());
			option4.displayAlignment = static_cast<Qt::Alignment>(1 << align);
		}
		std::vector<QString> const & column_elides = m_log_widget.config().m_columns_elide;
		if (index.column() < column_elides.size())
		{
			E_Elide const elide = stringToElide(column_elides[index.column()].at(0).toLatin1());
			option4.textElideMode = static_cast<Qt::TextElideMode>(elide);
		}

		int const tag = m_log_widget.column2Tag(index.column());
		if (tag < tlv::tag_max_value)
		{
			switch (tag)
			{
				case tlv::tag_file:
					paintTokenized(painter, option4, index, QString("[:/\\\\]"), "/", m_log_widget.config().m_cut_path_level);
					break;
				case tlv::tag_func:
					paintTokenized(painter, option4, index, QString("[::]"), "::",  m_log_widget.config().m_cut_namespaces);
					break;
				case tlv::tag_ctx:
					if (m_app_data.getDictCtx().m_names.size())
						paintContext(painter, option4, index);
					else
						QStyledItemDelegate::paint(painter, option4, index); // no dictionnary
					break;
				case tlv::tag_ctime:
				case tlv::tag_stime:
					paintTime(painter, option4, index);
					break;
				
				default:
					QStyledItemDelegate::paint(painter, option4, index);
					break;
			}
		}
		else
		{
			// user tags
			QStyledItemDelegate::paint(painter, option4, index);
		}

		painter->restore();
	}
}


