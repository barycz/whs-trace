#include "connection.h"
#include <QListView>
#include <QFile>
#include <QRegExp>
#include <tlv_parser/tlv_encoder.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include "filterproxy.h"
#include "modelview.h"
#include "qtsln/qtcolorpicker/qtcolorpicker.h"

void Connection::onInvalidateFilter ()
{
	if (isModelProxy())
		static_cast<FilterProxyModel *>(m_table_view_proxy)->force_update();
	else
	{
		ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		model->emitLayoutChanged();
	}
}

void Connection::setFilterFile (int state)
{
	if (state == Qt::Unchecked)
	{
		m_table_view_widget->setModel(m_table_view_proxy->sourceModel());
	}
	else if (state == Qt::Checked)
	{
		if (!m_table_view_proxy)
		{
			m_table_view_proxy = new FilterProxyModel(this, m_session_state);

			m_table_view_proxy->setSourceModel(m_table_view_widget->model());
			m_table_view_widget->setModel(m_table_view_proxy);
		}
		else
		{
			m_table_view_widget->setModel(m_table_view_proxy);
		}

		static_cast<FilterProxyModel *>(m_table_view_proxy)->force_update();
		onInvalidateFilter();
	}

	m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetCtx()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetTID()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetColorRegex()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetRegex()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetLvl()->setEnabled(m_main_window->filterEnabled());
	if (m_column_setup_done)
		setupColumnSizes(true);
}

void Connection::clearFilters (QStandardItem * node)
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
}

void Connection::clearFilters ()
{
	//@TODO: call all functions below
	//sessionState().clearFilters();
}

void Connection::onClearCurrentFileFilter ()
{
	sessionState().onClearFileFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentCtxFilter ()
{
	sessionState().onClearCtxFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentTIDFilter ()
{
	sessionState().onClearTIDFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentColorizedRegexFilter ()
{
	sessionState().onClearColorizedRegexFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentRegexFilter ()
{
	sessionState().onClearRegexFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentScopeFilter ()
{
	sessionState().onClearScopeFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentRefTime ()
{
	sessionState().onClearRefTime();
	onInvalidateFilter();
}

void Connection::onExcludeFileLine (QModelIndex const & row_index)
{
	QString file = findString4Tag(tlv::tag_file, row_index);
	QString line = findString4Tag(tlv::tag_line, row_index);
	qDebug("appending: %s:%s", file.toStdString().c_str(), line.toStdString().c_str());
	std::string const fileline = file.toStdString() + "/" + line.toStdString();
	m_file_model->stateToItem(fileline, Qt::Unchecked);
	onInvalidateFilter();
}

void Connection::onFileColOrExp (QModelIndex const & idx, bool collapsed)
{
	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(m_main_window->getWidgetFile()->model());
	QStandardItem * const node = model->itemFromIndex(idx);

	std::vector<QString> s;	// @TODO: hey piggy, to member variables
	s.clear();
	s.reserve(16);
	QStandardItem * parent = node;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	std::string file;
	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += std::string("/") + (*it).toStdString();

	sessionState().m_file_filters.set_to_state(file, TreeModelItem(static_cast<E_NodeStates>(node->checkState()), collapsed));
}

void Connection::onFileExpanded (QModelIndex const & idx)
{
	onFileColOrExp(idx, false);
}

void Connection::onFileCollapsed (QModelIndex const & idx)
{
	onFileColOrExp(idx, true);
}

void Connection::appendToTIDFilters (std::string const & item)
{
	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_tid_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, Qt::Checked);
		root->appendRow(row_items);
	}
}

void Connection::appendToLvlWidgets (FilteredLevel const & flt)
{
	QStandardItem * root = m_lvl_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, flt.m_level_str);
	if (child == 0)
	{
		E_LevelMode const mode = static_cast<E_LevelMode>(flt.m_state);
		QList<QStandardItem *> row_items = addTriRow(flt.m_level_str, Qt::Checked, lvlModToString(mode));
		row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
		m_main_window->getWidgetLvl()->sortByColumn(0, Qt::AscendingOrder);
	}
}

void Connection::appendToLvlFilters (std::string const & item)
{
	bool enabled = false;
	E_LevelMode lvlmode = e_LvlInclude;
	if (sessionState().isLvlPresent(item, enabled, lvlmode))
		return;

	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_lvl_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Checked, true);
		row_items[0]->setCheckState(Qt::Checked);
		root->appendRow(row_items);
		m_main_window->getWidgetLvl()->sortByColumn(0, Qt::AscendingOrder);

		sessionState().appendLvlFilter(qItem.toStdString());
	}
}

void Connection::appendToCtxWidgets (FilteredContext const & flt)
{
	QStandardItem * root = m_ctx_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, flt.m_ctx_str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(flt.m_ctx_str, Qt::Checked);
		row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}


void Connection::appendToCtxFilters (std::string const & item, bool checked)
{
	bool enabled = false;
	if (sessionState().isCtxPresent(item, enabled))
		return;

	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_ctx_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, Qt::Checked);
		row_items[0]->setCheckState(Qt::Checked);
		root->appendRow(row_items);
		sessionState().appendCtxFilter(qItem.toStdString());
	}
}

bool Connection::appendToFilters (DecodedCommand const & cmd)
{
	std::string line;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_line)
		{
			line = cmd.tvs[i].m_val;
			break;
		}

		if (cmd.tvs[i].m_tag == tlv::tag_tid)
		{
			int const idx = sessionState().m_tls.findThreadId(cmd.tvs[i].m_val);
			if (cmd.hdr.cmd == tlv::cmd_scope_entry)
				sessionState().m_tls.incrIndent(idx);
			if (cmd.hdr.cmd == tlv::cmd_scope_exit)
				sessionState().m_tls.decrIndent(idx);
			appendToTIDFilters(cmd.tvs[i].m_val);
		}

		if (cmd.tvs[i].m_tag == tlv::tag_ctx)
		{
			appendToCtxFilters(cmd.tvs[i].m_val, false);
		}
		if (cmd.tvs[i].m_tag == tlv::tag_lvl)
		{
			appendToLvlFilters(cmd.tvs[i].m_val);
		}
	}

	boost::char_separator<char> sep(":/\\");

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			std::string file(cmd.tvs[i].m_val);
			QModelIndex const ret = m_file_model->insertItem(file + "/" + line);
			if (ret.isValid())
				m_main_window->getWidgetFile()->hideLinearParents();
		}
	}
	return true;
}

void Connection::appendToRegexFilters (std::string const & str, bool checked, bool inclusive)
{
	m_session_state.appendToRegexFilters(str, checked, inclusive);
}

void Connection::removeFromRegexFilters (std::string const & val)
{
	m_session_state.removeFromRegexFilters(val);
}

void Connection::recompileRegexps ()
{
	for (int i = 0, ie = sessionState().m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = sessionState().m_filtered_regexps[i];
		QStandardItem * root = m_regex_model->invisibleRootItem();
		QString const qregex = QString::fromStdString(fr.m_regex_str);
		QStandardItem * child = findChildByText(root, qregex);
		fr.m_is_enabled = false;
		if (!child)
			continue;
		QRegExp regex(qregex);
		if (regex.isValid())
		{
			fr.m_regex = regex;
			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				child->setToolTip(tr("ok"));
				fr.m_is_enabled = true;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				child->setToolTip(tr("regex not enabled"));
			}
		}
		else
		{
			if (child)
			{
				child->setData(QBrush(Qt::red), Qt::BackgroundRole);
				child->setToolTip(regex.errorString());
			}
		}
	}

	onInvalidateFilter();
}

void Connection::appendToColorRegexFilters (std::string const & val)
{
	m_session_state.appendToColorRegexFilters(val);
}

void Connection::removeFromColorRegexFilters (std::string const & val)
{
	m_session_state.removeFromColorRegexFilters(val);
}

void Connection::loadToColorRegexps (std::string const & filter_item, std::string const & color, bool enabled)
{
	sessionState().appendToColorRegexFilters(filter_item);
	sessionState().setRegexColor(filter_item, QColor(color.c_str()));
	sessionState().setRegexChecked(filter_item, enabled);
}

void Connection::onColorRegexChanged ()
{
	for (int i = 0, ie = sessionState().m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = sessionState().m_colorized_texts[i];
		QStandardItem * root = m_color_regex_model->invisibleRootItem();
		QString const qregex = QString::fromStdString(ct.m_regex_str);
		QStandardItem * child = findChildByText(root, qregex);
		QModelIndex const idx = m_color_regex_model->indexFromItem(child);
		if (!child)
			continue;

		if (QtColorPicker * w = static_cast<QtColorPicker *>(m_main_window->getWidgetColorRegex()->indexWidget(idx)))
		{
			ct.m_qcolor = w->currentColor();
		}
	}
	onInvalidateFilter();
}

void Connection::recompileColorRegexps ()
{
	for (int i = 0, ie = sessionState().m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = sessionState().m_colorized_texts[i];
		QStandardItem * root = m_color_regex_model->invisibleRootItem();
		QString const qregex = QString::fromStdString(ct.m_regex_str);
		QStandardItem * child = findChildByText(root, qregex);
		QModelIndex const idx = m_color_regex_model->indexFromItem(child);
		ct.m_is_enabled = false;
		if (!child)
			continue;

		if (m_main_window->getWidgetColorRegex()->indexWidget(idx) == 0)
		{
			QtColorPicker * w = new QtColorPicker(m_main_window->getWidgetColorRegex(), qregex);
			w->setStandardColors();
			w->setCurrentColor(ct.m_qcolor);

			connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onColorRegexChanged()));
			m_main_window->getWidgetColorRegex()->setIndexWidget(idx, w);
		}
		else
		{
			QtColorPicker * w = static_cast<QtColorPicker *>(m_main_window->getWidgetColorRegex()->indexWidget(idx));
			w->setCurrentColor(ct.m_qcolor);
		}

		QRegExp regex(qregex);
		if (regex.isValid())
		{
			ct.m_regex = regex;

			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				child->setToolTip(tr("ok"));
				ct.m_is_enabled = true;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				child->setToolTip(tr("not checked"));
			}
		}
		else
		{
			if (child)
			{
				child->setData(QBrush(Qt::red), Qt::BackgroundRole);
				child->setToolTip(regex.errorString());
			}
		}
	}

	onInvalidateFilter();
}

void Connection::loadToRegexps (std::string const & filter_item, bool inclusive, bool enabled)
{
	sessionState().appendToRegexFilters(filter_item, inclusive, enabled);
}

