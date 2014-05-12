#include "baseproxymodel.h"
#include <connection.h>
#include <mainwindow.h>

BaseProxyModel::BaseProxyModel (QObject * parent)
	: QAbstractProxyModel(parent)
{ }

void BaseProxyModel::force_update ()
{
	emit layoutAboutToBeChanged();
	QAbstractTableModel const * src_model = static_cast<QAbstractTableModel const *>(sourceModel());

	// rows
	m_map_from_tgt.clear();
	m_map_from_src.clear();
	m_map_from_src.reserve(src_model->rowCount());
	int tgt_idx = 0;
	for (int src_idx = 0, se = src_model->rowCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsRow(src_idx, QModelIndex()))
		{
			m_map_from_src.insert(std::make_pair(src_idx, tgt_idx));
			++tgt_idx;
		}
	}

	m_map_from_tgt.resize(m_map_from_src.size());
	for (map_t::const_iterator it = m_map_from_src.begin(), ite = m_map_from_src.end(); it != ite; ++it)
		m_map_from_tgt[it->second] = it->first;

	// cols
	m_cmap_from_tgt.clear();
	m_cmap_from_src.clear();
	m_cmap_from_src.reserve(src_model->columnCount());
	int ctgt_idx = 0;
	for (int src_idx = 0, se = src_model->columnCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsColumn(src_idx, QModelIndex()))
		{
			m_cmap_from_src.insert(std::make_pair(src_idx, ctgt_idx));
			++ctgt_idx;
		}
	}

	m_cmap_from_tgt.resize(m_cmap_from_src.size());
	for (map_t::const_iterator it = m_cmap_from_src.begin(), ite = m_cmap_from_src.end(); it != ite; ++it)
	{
		m_cmap_from_tgt[it->second] = it->first;
	}

	emit layoutChanged();
}

QVariant BaseProxyModel::data (QModelIndex const & index, int role) const
{
	QModelIndex const src_idx = mapToSource(index);
	return sourceModel()->data(src_idx, role);
}

void BaseProxyModel::insertCol (int c)
{
	int pos0 = 0;
	int pos1 = static_cast<int>(m_cmap_from_src.size());
	map_t::iterator it0 = m_cmap_from_src.lower_bound(c);
	map_t::iterator it1 = m_cmap_from_src.upper_bound(c);

	if (it0 != m_cmap_from_src.end() && it0->first == c)
		return;

	if (it0 != m_cmap_from_src.end())
		pos0 = it0->second;
	if (it1 != m_cmap_from_src.end())
	{
		pos1 = it1->second;
		for (map_t::iterator iit = it1, iite = m_cmap_from_src.end(); iit != iite; ++iit)
		{
			iit->second += 1;
		}
	}
	//qDebug("  pxy|  beginInsertCols^^^(%02i, %02i)", pos0, pos1);
	beginInsertColumns(QModelIndex(), pos1, pos1);
	m_cmap_from_src.insert(std::make_pair(c, pos1));

	m_cmap_from_tgt.resize(m_cmap_from_src.size()); // ugh
	for (map_t::const_iterator it = m_cmap_from_src.begin(), ite = m_cmap_from_src.end(); it != ite; ++it)
	{
		m_cmap_from_tgt[it->second] = it->first;
	}
	endInsertColumns();
}

void BaseProxyModel::insertRow (int c)
{
	int pos0 = 0;
	int pos1 = static_cast<int>(m_map_from_src.size());
	map_t::iterator it0 = m_map_from_src.lower_bound(c);
	map_t::iterator it1 = m_map_from_src.upper_bound(c);

	if (it0 != m_map_from_src.end() && it0->first == c)
	{
		//qDebug("row %i already present", c);
		return;
	}

	if (it0 != m_map_from_src.end())
		pos0 = it0->second;
	if (it1 != m_map_from_src.end())
	{
		pos1 = it1->second;
		for (map_t::iterator iit = it1, iite = m_map_from_src.end(); iit != iite; ++iit)
		{
			iit->second += 1;
		}
	}

	//qDebug("  pxy-  beginInsertRows===(%02i, %02i)", pos0, pos1);
	beginInsertRows(QModelIndex(), pos0, pos0);
	m_map_from_src.insert(std::make_pair(c, pos1));

	m_map_from_tgt.resize(m_map_from_src.size()); // ugh
	for (map_t::const_iterator it = m_map_from_src.begin(), ite = m_map_from_src.end(); it != ite; ++it)
	{
		m_map_from_tgt[it->second] = it->first;
	}
	endInsertRows();
}


bool BaseProxyModel::setData (QModelIndex const & src_index, QVariant const & value, int role)
{
	//qDebug("proxy setData: r=%i, c=%i", src_index.row(), src_index.column());
	if (filterAcceptsColumn(src_index.column(), QModelIndex()))
		insertCol(src_index.column());

	if (filterAcceptsRow(src_index.row(), QModelIndex()))
		insertRow(src_index.row());

	QModelIndex const index = mapFromSource(src_index);
	if (!index.isValid()) return false;

	emit dataChanged(index, index);
	return true;
}


/*QVariant BaseProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	//int const tgt_idx = colFromSource(section);
	return QVariant();
}*/

/*bool  BaseProxyModel::setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role)
{
	int const tgt_idx = colFromSource(section);
	return sourceModel()->setHeaderData(tgt_idx, orientation, value, role);
}*/


Qt::ItemFlags BaseProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex BaseProxyModel::index (int row, int column, QModelIndex const & parent) const
{
	if (row < static_cast<int>(m_map_from_tgt.size())
		&& column < static_cast<int>(m_cmap_from_tgt.size()))
		return QAbstractItemModel::createIndex(row, column);
	return QModelIndex();
}

int BaseProxyModel::rowCount (QModelIndex const & parent) const
{
	return static_cast<int>(m_map_from_tgt.size());
}

int BaseProxyModel::columnCount (QModelIndex const & parent) const
{
	return static_cast<int>(m_cmap_from_tgt.size());
}

bool BaseProxyModel::rowInProxy (int row) const
{
	map_t::const_iterator it = m_map_from_src.find(row);
	if (it != m_map_from_src.end())
		return true;
	return false;
}
int BaseProxyModel::rowToSource (int row) const
{
	if (row < static_cast<int>(m_map_from_tgt.size()))
		return m_map_from_tgt[row];
	return -1;
}

bool BaseProxyModel::colInProxy (int col) const
{
	return colToSource(col) > -1;
}

int BaseProxyModel::colToSource (int col) const
{
	if (col < static_cast<int>(m_cmap_from_tgt.size()))
		return m_cmap_from_tgt[col];
	return -1;
}

int BaseProxyModel::colFromSource (int col) const
{
	map_t::const_iterator cit = m_cmap_from_src.find(col);
	if (cit != m_cmap_from_src.end())
		return cit->second;
	return -1;
}

QModelIndex BaseProxyModel::mapToSource (QModelIndex const & proxyIndex) const
{
	if (proxyIndex.isValid())
		if (proxyIndex.row() < static_cast<int>(m_map_from_tgt.size()))
			if (proxyIndex.column() < static_cast<int>(m_cmap_from_tgt.size()))
			return QAbstractItemModel::createIndex(m_map_from_tgt[proxyIndex.row()], m_cmap_from_tgt[proxyIndex.column()]);
	return QModelIndex();
}

QModelIndex BaseProxyModel::mapFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid())
	{
		map_t::const_iterator rit = m_map_from_src.find(sourceIndex.row());
		map_t::const_iterator cit = m_cmap_from_src.find(sourceIndex.column());
		if (cit != m_cmap_from_src.end() && rit != m_map_from_src.end())
			return QAbstractItemModel::createIndex(rit->second, cit->second);
	}
	return QModelIndex();
}

QModelIndex BaseProxyModel::mapNearestFromSource (QModelIndex const & sourceIndex) const
{
	map_t::const_iterator const row_it = m_map_from_src.find(sourceIndex.row());
	int d = std::numeric_limits<int>::max();
	map_t::const_iterator nearest = m_cmap_from_src.end();
	for (map_t::const_iterator it = m_cmap_from_src.begin(), ite = m_cmap_from_src.end(); it != ite; ++it)
	{
		int dist = abs(it->first - sourceIndex.column());
		if (dist < d)
		{
			d = dist;
			nearest = it;
		}
	}

	if (nearest != m_cmap_from_src.end() && row_it != m_map_from_src.end())
	{
		return QAbstractItemModel::createIndex(row_it->second, nearest->second);
	}
	return QModelIndex();
}


bool BaseProxyModel::insertRows (int first, int last, QModelIndex const & parent)
{
	return true;
}

bool BaseProxyModel::insertColumns (int first, int last, QModelIndex const & parent)
{
	return true;
}

void BaseProxyModel::insertAllowedColumn (int src_col)
{
	if (src_col >= m_allowed_src_cols.size())
		m_allowed_src_cols.resize(src_col + 1);
	m_allowed_src_cols[src_col] = 1;
}

void BaseProxyModel::removeAllowedColumn (int src_col)
{
	if (src_col >= m_allowed_src_cols.size())
		m_allowed_src_cols.resize(src_col + 1);
	m_allowed_src_cols[src_col] = 0;
}

