#include "dockmanagermodel.h"
#include <QString>
#include <QMainWindow>
#include "dockmanager.h"
#include "dock.h"

DockManagerModel::DockManagerModel (DockManager & mgr, QObject * parent, tree_data_t * data)
	: TreeModel<DockedInfo>(parent, data)
	, m_manager(mgr)
{
	qDebug("%s", __FUNCTION__);
}

DockManagerModel::~DockManagerModel ()
{
	qDebug("%s", __FUNCTION__);
}
QModelIndex DockManagerModel::testItemWithPath (QStringList const & path)
{
	QString const name = path.join("/");
	DockedInfo const * i = 0;
	if (node_t const * node = m_tree_data->is_present(name, i))
		return indexFromItem(node);
	else
		return QModelIndex();
}

QModelIndex DockManagerModel::insertItemWithPath (QStringList const & path, bool checked)
{
	QString const name = path.join("/");
	DockedInfo const * i = 0;
	node_t const * node = m_tree_data->is_present(name, i);
	if (node)
	{
		//qDebug("%s path=%s already present", __FUNCTION__, path.toStdString().c_str());
		return indexFromItem(node);
	}
	else
	{
		//qDebug("%s path=%s not present, adding", __FUNCTION__, path.toStdString().c_str());
		DockedInfo i;
		i.m_state = checked ? Qt::Checked : Qt::Unchecked;
		i.m_collapsed = false;
		//i.m_path = path;
	
		node_t * const n = m_tree_data->set_to_state(name, i);
		QModelIndex const parent_idx = indexFromItem(n->parent);
		int const last = n->parent->count_childs() - 1;
		beginInsertRows(parent_idx, last, last);
		n->data.m_path = path;

		QModelIndex const idx = indexFromItem(n);
		endInsertRows();
		initData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
		//QModelIndex const parent_idx = idx.parent();
		//if (parent_idx.isValid())
		//	emit dataChanged(parent_idx, parent_idx);
		return idx;
	}
}

int DockManagerModel::columnCount (QModelIndex const & parent) const
{
	return DockManager::e_max_dockmgr_column;
}

Qt::ItemFlags DockManagerModel::flags (QModelIndex const & index) const
{
	return QAbstractItemModel::flags(index)
				| Qt::ItemIsEnabled
				| Qt::ItemIsUserCheckable
				| Qt::ItemIsSelectable;
}

/*DockedWidgetBase const * DockManagerModel::getWidgetFromIndex (QModelIndex const & index) const
{
	DockManager const * const mgr = static_cast<DockManager const *>(QObject::parent());
	node_t const * const item = itemFromIndex(index);
	QStringList const & p = item->data.m_path;
	DockedWidgetBase const * const dwb = m_manager.findDockable(p.join("/"));
	return dwb;
}

DockedWidgetBase * DockManagerModel::getWidgetFromIndex (QModelIndex const & index)
{
	DockManager * const mgr = static_cast<DockManager *>(QObject::parent());
	node_t const * const item = itemFromIndex(index);
	QStringList const & p = item->data.m_path;
	DockedWidgetBase * dwb = m_manager.findDockable(p.join("/"));
	return dwb;
}*/

QVariant DockManagerModel::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int const col = index.column();
	if (col == DockManager::e_Column_Name)
		return TreeModel<DockedInfo>::data(index, role);
	return QVariant();
}

bool DockManagerModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;

	if (role == Qt::CheckStateRole)
	{
		node_t const * const n = itemFromIndex(index);
		QStringList const & dst = n->data.m_path;

		Action a;
		a.m_type = e_Visibility;
		a.m_src_path = m_manager.path();
		a.m_src = &m_manager;
		a.m_dst_path = dst;

		int const state = value.toInt();
		a.m_args.push_back(state);
		m_manager.handleAction(&a, e_Sync);
	}

	return TreeModel<DockedInfo>::setData(index, value, role);
}

bool DockManagerModel::initData (QModelIndex const & index, QVariant const & value, int role)
{
	return TreeModel<DockedInfo>::setData(index, value, role);
}




