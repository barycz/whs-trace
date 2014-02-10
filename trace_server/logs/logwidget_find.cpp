#include "logwidget.h"
#include <QStatusBar>
#include "logs/logtablemodel.h"
#include <logs/filterproxymodel.h>
#include <logs/findproxymodel.h>
#include "utils.h"
#include "connection.h"
#include "warnimage.h"

namespace logs {

void LogWidget::onFind ()
{
	//m_find_widget->onCancel();
	//w->setFocusProxy(m_find_widget);
	//m_find_widget->setFocusProxy(w); // dunno what the proxies are for
	//mk_action configure find widget
	FindConfig & cfg = m_config.m_find_config;
	m_find_widget->applyConfig(cfg);

	m_find_widget->onActivate();

	//setFocusProxy(m_find_widget);
	m_find_widget->setFocusProxy(this); // dunno what the proxies are for
}

void LogWidget::onFindNext ()
{
	m_find_widget->onFindNext();
}

void LogWidget::onFindPrev ()
{
	m_find_widget->onFindPrev();
}

void LogWidget::onFindAllRefs ()
{
	m_find_widget->onFindAllRefs();
}

void LogWidget::findInWholeTable (FindConfig const & fc, QModelIndexList & result)
{
	for (int i = 0, ie = model()->rowCount(); i < ie; ++i)
	{
		for (int j = 0, je = model()->columnCount(); j < je; ++j)
		{
			QModelIndex const idx = model()->index(i, j, QModelIndex());
			QString s = model()->data(idx).toString();
			if (matchToFindConfig(s, fc))
				result.push_back(idx);
		}
	}
}

void LogWidget::registerLinkedWidget (DockedWidgetBase * l)
{
	if (std::find(m_linked_widgets.begin(), m_linked_widgets.end(), l) == m_linked_widgets.end())
		m_linked_widgets.push_back(l);
}

void LogWidget::unregisterLinkedWidget (DockedWidgetBase * l)
{
	m_linked_widgets.erase(std::remove(m_linked_widgets.begin(), m_linked_widgets.end(), l), m_linked_widgets.end());
}

void LogWidget::linkToSource (DockedWidgetBase * src)
{
	m_linked_parent = src;
}

void LogWidget::setSrcModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	setModel(m_src_model);
	m_src_model->resizeToCfg(m_config);
	m_find_proxy_selection = new QItemSelectionModel(m_find_proxy_model);
	setSelectionModel(m_src_selection);
	resizeSections();
	applyConfig();
}

void LogWidget::setFindProxyModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	setModel(m_find_proxy_model);
	m_find_proxy_model->resizeToCfg(m_config);
	m_find_proxy_model->force_update();
	resizeSections();
	applyConfig();
}

LogWidget * LogWidget::mkFindAllRefsLogWidget (FindConfig const & fc)
{
	QString tag;
	if (fc.m_to_widgets.isEmpty())
		tag = "find_all_refs";
	else
	{
		// @TODO: validate widget form
		tag = fc.m_to_widgets.at(0);
	}

	LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = m_connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	if (!loaded)
	{
		cfg = m_config;	// inherit from parent widget if not loaded
	}
	cfg.m_tag = tag;
	cfg.m_show = true;
	cfg.m_central_widget = false;
	cfg.m_filter_proxy = false;
	cfg.m_find_proxy = true;
	datalogs_t::iterator it = m_connection->dataWidgetFactoryFrom<e_data_log>(tag, cfg);

	DataLog * dp = *it;
	DataLog::widget_t & child_outer = dp->widget();
	LogWidget & child = *child_outer.m_lw;
	child.linkToSource(m_dwb);
	registerLinkedWidget(child.m_dwb);
	child.loadAuxConfigs();

	child.setupLogModel(m_src_model);
	child.setFindProxyModel(fc);
	child.m_kfind_proxy_selection = new KLinkItemSelectionModel(child.model(), selectionModel());
	child.setSelectionModel(child.m_kfind_proxy_selection);
	child.applyConfig();
	child.m_config_ui.refreshUI();
	child.refreshFilters(child.m_find_proxy_model);

	dp->m_wd->setStyleSheet("\
			QHeaderView::section {\
			background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,\
											  stop:0 #616161, stop: 0.5 #505050,\
											  stop: 0.6 #434343, stop:1 #656565);\
			color: yellow;\
			padding-left: 4px;\
			border: 1px solid #6c6c6c;\
		}");

	//m_connection->getMainWindow()->onDockRestoreButton();
	return &child;
}

LogWidget * LogWidget::mkFindAllCloneLogWidget (FindConfig const & fc)
{
	QString tag;
	if (fc.m_to_widgets.isEmpty())
	{
		if (fc.m_refs)
			tag = "find_all_refs";
		else if (fc.m_clone)
			tag = "find_all_clone";
		else
			tag = "noname";
	}
	else
	{
		// @TODO: validate widget form: appname/foo
		tag = fc.m_to_widgets.at(0);
	}

	LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = m_connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	if (!loaded)
	{
		cfg = m_config;	// inherit from parent widget if not loaded
	}
	cfg.m_tag = tag;
	cfg.m_show = true;
	cfg.m_central_widget = false;
	cfg.m_filter_proxy = false;
	cfg.m_find_proxy = false;
	datalogs_t::iterator it = m_connection->dataWidgetFactoryFrom<e_data_log>(tag, cfg);

	DataLog * dp = *it;
	DataLog::widget_t & child_outer = dp->widget();
	LogWidget & child = *child_outer.m_lw;
	child.loadAuxConfigs();
	LogTableModel * clone_model = cloneToNewModel(fc);
	child.setupLogModel(clone_model);
	child.setSrcModel(fc);
	child.m_config_ui.refreshUI();

	return &child;
}

void LogWidget::findAndSelect (FindConfig const & fc)
{
	QModelIndex start = model()->index(0, 0);

	// if selected column
	//	QModelIndexList indexes = model()->match(start, Qt::DisplayRole, QVariant(fc.m_str), -1, Qt::MatchFixedString);
	QModelIndexList indexes;
	findInWholeTable(fc, indexes);

	QItemSelectionModel * selection_model = selectionModel();

	QItemSelection selection;
	foreach(QModelIndex index, indexes) {

		QModelIndex left = model()->index(index.row(), 0);
		QModelIndex right = model()->index(index.row(), model()->columnCount() - 1);

		QItemSelection sel(left, right);
		selection.merge(sel, QItemSelectionModel::Select);
	}
	selection_model->select(selection, QItemSelectionModel::Select);
}

void LogWidget::currSelection (QModelIndexList & sel) const
{
	sel.clear();
	QItemSelectionModel const * selection = selectionModel();
	QItemSelection const sele = selection->selection();
	QModelIndexList const & sel2 = sele.indexes();
	//QModelIndexList sel2  = selection->selectedIndexes();
	if (sel2.size() < 1)
		return;
	/*
	QAbstractItemModel * m = model();
	QItemSelectionModel * selection = selectionModel();
	if (!selection)
		return QString();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	std::sort(indexes.begin(), indexes.end());*/

	// @FIXME: when proxy is on, there are invalid indexes in the current selection
	for(int i = 0, ie = sel2.size(); i < ie; ++i)
	{
		if (sel2.at(i).isValid())
			sel.push_back(sel2.at(i));
	}

	std::sort(sel.begin(), sel.end());
}

void LogWidget::findAndSelectNext (FindConfig const & fc)
{
	QModelIndexList l;
	currSelection(l);
	if (l.size())
	{
		QModelIndex const & curr_idx = l.at(l.size() - 1);
		QModelIndex const idx = model()->index(curr_idx.row() + 1, curr_idx.column(), QModelIndex());
		if (!idx.isValid())
		{
			noMoreMatches();
			return;
		}

		QModelIndexList next;
		for (int i = idx.row(), ie = model()->rowCount(); i < ie; ++i)
		{
			for (int j = 0, je = model()->columnCount(); j < je; ++j)
			{
				QModelIndex const idx = model()->index(i, j, QModelIndex());
				QString s = model()->data(idx).toString();
				if (matchToFindConfig(s, fc))
				{
					next.push_back(idx);
					break;
				}
			}
			if (next.size() > 0)
				break;
		}

		if (next.size() == 0)
		{
			noMoreMatches();
		}
		else
		{
			QItemSelectionModel * selection_model = selectionModel();
			QItemSelection selection;
			foreach(QModelIndex index, next)
			{
				QModelIndex left = model()->index(index.row(), 0);
				QModelIndex right = model()->index(index.row(), model()->columnCount() - 1);

				QItemSelection sel(left, right);
				selection.merge(sel, QItemSelectionModel::Select);
			}
			selection_model->clearSelection();
			selection_model->select(selection, QItemSelectionModel::Select);
			scrollTo(next.at(0), QTableView::PositionAtCenter);
		}
	}
}

void LogWidget::findAndSelectPrev (FindConfig const & fc)
{
	QModelIndexList l;
	currSelection(l);
	if (l.size())
	{
		QModelIndex const & curr_idx = l.at(0);
		QModelIndex const idx = model()->index(curr_idx.row() - 1, curr_idx.column(), QModelIndex());
		if (!idx.isValid())
		{
			noMoreMatches();
			return;
		}

		QModelIndexList next;
		/// ????
		for (int i = curr_idx.row(); i --> 0; )
		{
			for (int j = model()->columnCount(); j --> 0; )
			{
				QModelIndex const idx = model()->index(i, j, QModelIndex());
				QString s = model()->data(idx).toString();
				if (matchToFindConfig(s, fc))
				{
					next.push_back(idx);
					break;
				}
			}
			if (next.size() > 0)
				break;
		}

		if (next.size() == 0)
		{
			noMoreMatches();
		}
		else
		{
			QItemSelectionModel * selection_model = selectionModel();
			QItemSelection selection;
			foreach(QModelIndex index, next)
			{
				QModelIndex left = model()->index(index.row(), 0);
				QModelIndex right = model()->index(index.row(), model()->columnCount() - 1);

				QItemSelection sel(left, right);
				selection.merge(sel, QItemSelectionModel::Select);
			}
			selection_model->clearSelection();
			selection_model->select(selection, QItemSelectionModel::Select);
			scrollTo(next.at(0), QTableView::PositionAtCenter);
		}
	}
}

void LogWidget::handleFindAction (FindConfig const & fc)
{
	bool const select_only = !fc.m_refs && !fc.m_clone;

	if (fc.m_regexp)
	{
		if (fc.m_regexp_val.isEmpty())
			return;
		if (!fc.m_regexp_val.isValid())
			return;
	}

	saveFindConfig();

	if (select_only)
	{
		if (fc.m_next)
			findAndSelectNext(fc);
		else if (fc.m_prev)
			findAndSelectPrev(fc);
		else
			findAndSelect(fc);
	}
	else
	{
		LogWidget * result_widget = 0;
		if (fc.m_refs)
		{
			result_widget = mkFindAllRefsLogWidget(fc);
		}
		else // clone
		{
			result_widget = mkFindAllCloneLogWidget(fc);
		}
	}
}

void LogWidget::noMoreMatches ()
{
	qDebug("end of search");
	m_connection->getMainWindow()->statusBar()->showMessage(tr("End of document!"));
	m_last_search_row = 0;

	// flash icon
	QPoint const global = rect().center();
	QPoint const pos(global.x() - m_warnimage->width() / 2, global.y() - m_warnimage->height() / 2);
    m_warnimage->setParent(this);
    m_warnimage->move(pos);
	m_warnimage->show();
	m_warnimage->activateWindow();
	m_warnimage->raise();
	m_warnimage->warningFindNoMoreMatches();
}

QString LogWidget::findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	return findVariant4Tag(tag, row_index).toString();
}

QVariant LogWidget::findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	int const idx = m_tags2columns[tag];
	if (idx == -1)
		return QVariant();

	LogTableModel * model = m_src_model;

	QModelIndex const model_idx = model->index(row_index.row(), idx, QModelIndex());
	if (model_idx.isValid())
	{
		QVariant value = model->data(model_idx);
		return value;
	}
	return QVariant();
}

void LogWidget::onFindFileLine (QModelIndex const &)
{
	//@FIXME: unused args
	qDebug("find file:line for idx=(%i,col)", m_last_clicked.row());

	bool const scroll_to_item = true;
	bool const expand = true;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);
}

}

