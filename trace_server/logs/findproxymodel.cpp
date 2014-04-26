#include "findproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
//#include <tlv_parser/tlv_encoder.h>
//#include <trace_client/trace.h>
//#include <connection.h>
#include "logwidget.h"
#include "logtablemodel.h"

FindProxyModel::FindProxyModel (QObject * parent, logs::LogWidget & lw)
	: FilterProxyModel(parent, lw)
{ }

FindProxyModel::~FindProxyModel ()
{
	qDebug("%s", __FUNCTION__);
}

bool FindProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	return true;
}

bool FindProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	if (sourceRow < m_log_widget.m_src_model->dcmds().size())
	{
		DecodedCommand const & dcmd = m_log_widget.m_src_model->dcmds()[sourceRow];

		for (size_t i = 0, ie = dcmd.m_tvs.size(); i < ie; ++i)
		{
			QString const & val = dcmd.m_tvs[i].m_val;
			FindConfig const & fc = m_log_widget.m_config.m_find_config;
			if (fc.m_regexp)
			{
			}
			else
			{
				Qt::CaseSensitivity const cs = fc.m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
				if (val.contains(fc.m_str, cs))
				{
					return true;
				}
			}
		}
		return false; // no match
	}
	qWarning("no dcmd for source row, should not happen!");
	return false;
}

