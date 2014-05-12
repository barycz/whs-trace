#include "logwidget.h"
#include <QListView>
#include <QFile>
#include <QRegExp>
#include <tlv_parser/tlv_encoder.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include "filterproxymodel.h"
#include <treeproxy.h>
#include "logs/logtablemodel.h"
#include "qtsln/qtcolorpicker/qtcolorpicker.h"

namespace logs {

void LogWidget::onInvalidateFilter ()
{
	QItemSelectionModel const * selection = m_tableview->selectionModel();
	if (!selection)
		return;

	QModelIndexList const old_selection = selection->selectedIndexes();

	//  fantomas ended here
	QModelIndexList srcs;
	if (isModelProxy())
	{
		for (int i = 0, ie = old_selection.size(); i < ie; ++i)
		{
			QModelIndex const & pxy_idx = old_selection.at(i);
			QModelIndex const src_idx = m_proxy_model->mapToSource(pxy_idx);

			//qDebug("update filter: pxy=(%2i, %2i) src=(%2i, %2i)", pxy_idx.row(), pxy_idx.column(), src_idx.row(), src_idx.column());
			srcs.push_back(src_idx);
		}
	}
	else
		srcs = old_selection;

	if (isModelProxy())
		static_cast<FilterProxyModel *>(m_proxy_model)->force_update();
	else
	{
		m_src_model->emitLayoutChanged();
	}

	syncSelection(srcs);

	scrollToCurrentTagOrSelection();
}

void LogWidget::syncSelection (QModelIndexList const & sel)
{
	m_tableview->selectionModel()->clearSelection();

	for (int i = 0, ie = sel.size(); i < ie; ++i)
	{
		QModelIndex idx = sel.at(i);
		if (isModelProxy())
		{
			idx = m_proxy_model->mapFromSource(sel.at(i));
			//qDebug("syncSelection: pxy src=(%2i, %2i)", idx.row(), idx.column());
		}
		//else qDebug("syncSelection: src=(%2i, %2i)", idx.row(), idx.column());

		if (idx.isValid())
			m_tableview->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
	}
}

/*void LogWidget::setFindProxyModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	setModel(m_find_proxy_model);
	m_find_proxy_model->force_update();
	m_find_proxy_model->resizeToCfg();
	resizeSections();
	applyConfig();
}*/


void LogWidget::setFilteringProxy (bool on)
{
	QItemSelectionModel const * selection = m_tableview->selectionModel();
	QModelIndexList indexes;
	if (selection)
		selection->selectedIndexes();

	if (!on)
	{
		qDebug("%s setting source model", __FUNCTION__);
		QModelIndexList srcs;
		if (m_proxy_model && indexes.size() > 0)
		{
			for (int i = 0, ie = indexes.size(); i < ie; ++i)
			{
				QModelIndex const & pxy_idx = indexes.at(i);
				QModelIndex const src_idx = m_proxy_model->mapToSource(pxy_idx);
				srcs.push_back(src_idx);
			}
		}

		m_tableview->setModel(m_src_model);

		if (srcs.size() > 0)
			m_tableview->setSelectionModel(m_src_selection);

		m_src_model->setProxy(0);

		for (int i = 0, ie = srcs.size(); i < ie; ++i)
			m_tableview->selectionModel()->setCurrentIndex(srcs.at(i), QItemSelectionModel::Select);
	}
	else
	{
		qDebug("%s setting proxy model", __FUNCTION__);
		m_tableview->setModel(m_proxy_model);
		// selection
		m_src_model->setProxy(m_proxy_model);

		if (m_proxy_model)
			m_proxy_model->setSourceModel(m_src_model);

		if (m_proxy_model)
			m_proxy_model->force_update();

		m_tableview->setSelectionModel(m_kproxy_selection);

		QModelIndexList pxys;
		for (int i = 0, ie = indexes.size(); i < ie; ++i)
		{
			QModelIndex const & src_idx = indexes.at(i);
			QModelIndex const pxy_idx = m_proxy_model->mapFromSource(src_idx);
			//qDebug("on: src(r=%i c=%i) -> pxy(r=%i c=%i)", src_idx.row(), src_idx.column(), pxy_idx.row(), pxy_idx.column());
			if (pxy_idx.isValid())
				pxys.push_back(pxy_idx);
		}

		for (int i = 0, ie = pxys.size(); i < ie; ++i)
		{
			//qDebug("on: pxy r=%i c=%i", pxys.at(i).row(), pxys.at(i).column());
			m_tableview->selectionModel()->setCurrentIndex(pxys.at(i), QItemSelectionModel::Select);
		}
	}

	if (m_config.m_in_view)
		scrollToCurrentTagOrSelection();
		//QTimer::singleShot(1000, this, SLOT(scrollToCurrentTagOrSelection()));
}

/*void LogWidget::clearFilters (QStandardItem * node)
{
	if (node)
	{
		if (node->checkState() == Qt::Checked)
		{
			node->setCheckState(Qt::Unchecked);
		}
		for (int i = 0, ie = node->rowCount(); i < ie; ++i)
			clearFilters(node->child(i));
	}
}*/

void LogWidget::refreshFilters (BaseProxyModel const * proxy)
{
	QAbstractItemModel * m = m_tableview->model();
	if (m && m == proxy)
	{
		Q_ASSERT(m_src_model);

		QModelIndexList src_list;
		for (int i = 0, ie = m_tableview->model()->rowCount(); i < ie; ++i)
		{
			QModelIndex const pxy_idx = m_tableview->model()->index(i, 0);
			QModelIndex const src_idx = proxy->mapToSource(pxy_idx);

			if (src_idx.isValid())
				src_list << src_idx;
		}
		foreach(QModelIndex index, src_list)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[index.row()];
			appendToFilters(dcmd);
		}
	}
}

void LogWidget::onRefillFilters ()
{
	if (m_tableview->model() == m_src_model || m_tableview->model() == m_proxy_model)
	{
		for (int i = 0, ie = m_src_model->dcmds().size(); i < ie; ++i)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[i];
			appendToFilters(dcmd);
			appendToColorizers(dcmd);
		}
	}
	else //if (model() == m_find_proxy_model)
	{
		//refreshFilters(m_find_proxy_model);
	}
}


void LogWidget::clearFilters ()
{
	//@TODO: call all functions below
	//m_filter_state.clearFilters();
}

void LogWidget::appendToTIDFilters (QString const & item)
{
	if (filterMgr()->getFilterTid())
	{
		QStandardItem * root = filterMgr()->getFilterTid()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, item);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(item, true);
			root->appendRow(row_items);
			filterMgr()->getFilterTid()->append(item);
		}
	}
}

/*void LogWidget::appendToLvlWidgets (FilteredLevel const & flt)
{
	if (filterMgr()->getFilterLvl())
	{
		QStandardItem * root = filterMgr()->getFilterLvl()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, flt.m_level_str);
		if (child == 0)
		{
			E_LevelMode const mode = static_cast<E_LevelMode>(flt.m_state);
			QList<QStandardItem *> row_items = addTriRow(flt.m_level_str, Qt::Checked, lvlModToString(mode));
			row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
			filterMgr()->getFilterLvl()->m_ui->view->sortByColumn(0, Qt::AscendingOrder);
		}
	}
}*/

void LogWidget::appendToLvlFilters (QString const & item)
{
	if (filterMgr()->getFilterLvl())
	{
		bool enabled = false;
		E_LevelMode lvlmode = e_LvlInclude;
		if (filterMgr()->getFilterLvl()->isPresent(item, enabled, lvlmode))
			return;

		QStandardItem * root = filterMgr()->getFilterLvl()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, item);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addTriRow(item, Qt::Checked, true);
			row_items[0]->setCheckState(Qt::Checked);
			root->appendRow(row_items);
			filterMgr()->getFilterLvl()->append(item);
		}
	}
}

/*void LogWidget::appendToCtxWidgets (FilteredContext const & flt)
{
	if (filterMgr()->getFilterCtx())
	{
		QStandardItem * root = filterMgr()->getFilterCtx()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, flt.m_ctx_str);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(flt.m_ctx_str, true);
			row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}*/


void LogWidget::appendToCtxFilters (QString const & item, bool checked)
{
	if (filterMgr()->getFilterCtx())
	{
		bool enabled = false;
		if (filterMgr()->getFilterCtx()->isCtxPresent(item, enabled))
			return;

		QStandardItem * root = filterMgr()->getFilterCtx()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, item);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(item, true);
			row_items[0]->setCheckState(Qt::Checked);
			root->appendRow(row_items);
			filterMgr()->getFilterCtx()->appendCtxFilter(item);
		}
	}
}

void LogWidget::appendToFileLineFilters (QString const & item)
{
	if (filterMgr()->getFilterFileLine())
	{
		filterMgr()->getFilterFileLine()->fileModel()->insertItem(item);
	}
		//void const * node = filterMgr()->getFilterFileLine()->fileModel()->insertItem(file + "/" + line);
		//batch.m_tree_node_ptrs.back() = node;
	//if (ret.isValid())
		//->hideLinearParents();
}

bool LogWidget::appendToFilters (DecodedCommand const & cmd)
{
	QString tid;
	if (cmd.getString(tlv::tag_tid, tid))
	{
		int const idx = m_tls.findThreadId(tid);
		if (cmd.m_hdr.cmd == tlv::cmd_scope_entry)
			m_tls.incrIndent(idx);
		if (cmd.m_hdr.cmd == tlv::cmd_scope_exit)
			m_tls.decrIndent(idx);
		appendToTIDFilters(tid);
	}

	QString ctx;
	if (cmd.getString(tlv::tag_ctx, ctx))
		appendToCtxFilters(ctx, false);

	QString lvl;
	if (cmd.getString(tlv::tag_lvl, lvl))
		appendToLvlFilters(lvl);

	QString file, line;
	if (cmd.getString(tlv::tag_line, line) && cmd.getString(tlv::tag_file, file))
		appendToFileLineFilters(file + "/" + line);
	return true;
}

void LogWidget::appendToRegexFilters (QString const & str, bool checked, bool inclusive)
{
	if (filterMgr()->getFilterRegex())
		filterMgr()->getFilterRegex()->appendToRegexFilters(str, checked, inclusive);
}

void LogWidget::removeFromRegexFilters (QString const & val)
{
	if (filterMgr()->getFilterRegex())
		filterMgr()->getFilterRegex()->removeFromRegexFilters(val);
}

void LogWidget::appendToStringFilters (QString const & str, bool checked, int state)
{
	if (filterMgr()->getFilterString())
	{
		filterMgr()->getFilterString()->appendToStringFilters(str, checked, state);
	}
}

void LogWidget::removeFromStringFilters (QString const & val)
{
	if (filterMgr()->getFilterString())
	{
		filterMgr()->getFilterString()->removeFromStringFilters(val);
	}
}


}

