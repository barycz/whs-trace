#include "connection.h"
#include "logs/logwidget.h"
#include "mainwindow.h"
#include "constants.h"
#include "utils.h"

namespace {
	void item2separator (QString const & item, QString & sep)
	{
		if (item == QLatin1String("\\t"))
			sep = QLatin1String("\t");
		else if (item == QLatin1String("\\n"))
			sep = QLatin1String("\n");
		else
			sep = item;
	}
}

bool Connection::handleCSVStreamCommand (DecodedCommand const & cmd)
{
	datalogs_t::iterator it = findOrCreateLog(m_app_name);
	(*it)->handleCommand(cmd, e_RecvBatched);
	// TODO
	/*if (!m_session_state.isConfigured())
	{
		if (m_session_state.separatorChar().isEmpty())
		{
			m_main_window->onSetupCSVSeparator(m_session_state.getAppIdx(), true);
			item2separator(m_main_window->separatorChar(), m_session_state.m_csv_separator);
		}

		QString const & val = cmd.tvs[0].m_val;
		QStringList const list = val.split(m_session_state.separatorChar());
		int const cols = list.size();

		int const idx = m_app_idx;
		if (m_main_window->m_config.m_columns_setup[idx].size() == 0)
		{
			m_main_window->m_config.m_columns_setup[idx].reserve(cols);
			m_main_window->m_config.m_columns_sizes[idx].reserve(cols);
			m_main_window->m_config.m_columns_align[idx].reserve(cols);
			m_main_window->m_config.m_columns_elide[idx].reserve(cols);

			if (idx >= 0 && idx < m_main_window->m_config.m_columns_setup.size())
				for (int i = 0; i < cols; ++i)
				{
					m_main_window->m_config.m_columns_setup[idx].push_back(tr("Col_%1").arg(i));
					m_main_window->m_config.m_columns_sizes[idx].push_back(127);
					m_main_window->m_config.m_columns_align[idx].push_back(QString(alignToString(e_AlignLeft)));
					m_main_window->m_config.m_columns_elide[idx].push_back(QString(elideToString(e_ElideRight)));
				}
		}

		m_main_window->onSetupCSVColumns(m_session_state.getAppIdx(), cols, true);
	}

	//appendToFilters(cmd);
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	model->appendCommandCSV(m_table_view_proxy, cmd);
	*/
	return true;
}


