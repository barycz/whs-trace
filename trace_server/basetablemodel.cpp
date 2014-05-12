#include "basetablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <trace_client/trace.h>
#include <sysfn/time_query.h>

BaseTableModel::BaseTableModel (QObject * parent, std::vector<QString> & hhdr, std::vector<int> & hsize)
	: QAbstractTableModel(parent)
	, m_column_count(0)
	, m_hhdr(hhdr)
	, m_hsize(hsize)
	, m_proxy(0)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	size_t const prealloc_size = 128;
	m_rows.reserve(prealloc_size);

	if (hsize.size() > 0)
	{
		int const last = static_cast<int>(hsize.size()) - 1;
		beginInsertColumns(QModelIndex(), m_column_count, last);
		insertColumns(m_column_count, last);
		m_column_count = last + 1;
		endInsertColumns();
	}
}

BaseTableModel::~BaseTableModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

int BaseTableModel::rowCount (QModelIndex const & /*parent*/) const { return static_cast<int>(m_rows.size()); }

int BaseTableModel::columnCount (QModelIndex const & /*parent*/) const
{
	return m_column_count;
}

inline bool BaseTableModel::checkExistence (QModelIndex const & index) const
{
	return index.row() < (int)m_rows.size() && index.column() < (int)m_rows[index.row()].size();
}

QVariant BaseTableModel::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	Cell const * c = 0;
	if (checkExistence(index))
	{
		c = &m_rows[index.row()][index.column()];
	}

	if (c == 0)
	{
		if (role == Qt::BackgroundRole)
			return QVariant::fromValue<QColor>(Qt::gray);
		return QVariant();
	}

	if (c && (role == Qt::DisplayRole || role == Qt::ToolTipRole))
	{
		return c->m_value;
	}
	else if (role == Qt::BackgroundRole)
	{
		if (c->m_bgc.isValid())
			return c->m_bgc;
		if (c->m_value.toString().isEmpty())
			return QVariant::fromValue<QColor>(Qt::gray);
		return QVariant();
	}
	else if (role == Qt::ForegroundRole)
	{
		return c->m_fgc;
	}
	return QVariant();
}

bool BaseTableModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;
	if (!checkExistence(index)) return false;

	int const x = index.column();
	int const y = index.row();
	if (role == Qt::DisplayRole)
		return false;
	else if (role == Qt::EditRole)
	{
		m_rows.at(y).at(x).m_value = value;
	}
	else if (role == Qt::BackgroundRole)
	{
		m_rows.at(y).at(x).m_bgc = value;
	}
	else if (role == Qt::ForegroundRole)
	{
		m_rows.at(y).at(x).m_fgc = value;
	}

	emit dataChanged(index, index);
	return true;
}


QVariant BaseTableModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
			case Qt::DisplayRole:
				if (section > -1 && section < m_hhdr.size())
					return m_hhdr.at(section);
				break;
			/*case Qt::SizeHintRole:
				if (section > -1 && section < m_hhdr.size())
				  return QSize(m_hsize[section], 12);
				return 128;*/
		}
	}
	return QVariant();
}

bool  BaseTableModel::setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role)
{
	if (section == -1)
		return false;
	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::EditRole)
		{
			if (section + 1 > m_hhdr.size())
			{
				m_hhdr.resize(section + 1);
			}
			m_hhdr[section] = value.toString();
			emit headerDataChanged(orientation, section, section);
		}
		else if (role == Qt::SizeHintRole)
		{
			if (section + 1 > m_hsize.size())
			{
				m_hsize.resize(section + 1);
			}
			m_hsize[section] = value.toInt();
			emit headerDataChanged(orientation, section, section);
		}
	}
	return true;
}

void BaseTableModel::emitLayoutChanged ()
{
	emit layoutChanged();
}

void BaseTableModel::clearModelData ()
{
	beginResetModel();

	removeRows(0, rowCount());
	m_row_ctimes.clear();
	m_row_stimes.clear();
	m_rows.clear();

	endResetModel();
}

void BaseTableModel::clearModel ()
{
	beginResetModel();

	m_row_ctimes.clear();
	m_row_stimes.clear();
	m_col_times.clear();
	m_rows.clear();
	m_column_count = 0;
	//m_hhdr.clear();
	//m_hsize.clear();
	removeRows(0, rowCount());
	removeColumns(0, columnCount());

	endResetModel();
}

void BaseTableModel::commitCommands (E_ReceiveMode mode)
{
	commitBatchToModel(m_batch);
}

void BaseTableModel::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	parseCommand(cmd, mode, m_batch);
	if (mode == e_RecvSync)
	{
		commitBatchToModel(m_batch);
		m_batch.clear();
	}
}

void BaseTableModel::createCell (unsigned long long time, int x, int y)
{
}

void BaseTableModel::createRows (unsigned long long time, int first, int last, QModelIndex const &)
{
	if (first >= m_rows.size())
	{
		//qDebug("+ model: y>rows.sz resize m_rows.sz=%i y=%i", m_rows.size(), first);
		beginInsertRows(QModelIndex(), m_rows.size(), first);
		size_t const curr_sz = m_rows.size();
		m_rows.resize(last + 1);
		m_row_ctimes.resize(last + 1);
		m_row_stimes.resize(last + 1);
		for (size_t r = curr_sz; r < last + 1; ++r)
		{
			m_row_ctimes[r] = time + r * 10;

			sys::hptimer_t const now = sys::queryTime_us();
			m_row_stimes[r] = now;
		}
		endInsertRows();
	}
}

void BaseTableModel::createColumns (unsigned long long time, int first, int last, QModelIndex const & )
{
	if (last >= m_rows.back().size())
	{
		m_rows.back().resize(last + 1);
		m_col_times.resize(last + 1);
	}

	if (m_column_count < last + 1)
	{
		beginInsertColumns(QModelIndex(), m_column_count, last);
		insertColumns(m_column_count, last);
		m_column_count = last + 1;
		endInsertColumns();
	}
}



