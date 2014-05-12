#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <QScrollBar>
#include <tlv_parser/tlv_encoder.h>
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include "delegates.h"
#include "mainwindow.h"
#include <cstdlib>

bool Connection::handleTableCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (getClosestFeatureState(e_data_table) == e_FtrDisabled)
		return true;

	QString msg_tag;
	if (!cmd.getString(tlv::tag_msg, msg_tag)) return true;
	QString tag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);
	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);

	datatables_t::iterator it = findOrCreateTable(tag);
	(*it)->handleCommand(cmd, mode);
	return true;
}

datatables_t::iterator Connection::findOrCreateTable (QString const & tag)
{
	typedef SelectIterator<e_data_table>::type iterator;
	iterator it = m_data.get<e_data_table>().find(tag);
	if (it == m_data.get<e_data_table>().end())
	{
		it = dataWidgetFactory<e_data_table>(tag);
		//(*it)->setupNewLogModel();
		(*it)->applyConfig(); // 0 means "create new model"

		// TMP!
/*		(*it)->m_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
		(*it)->m_widget->verticalHeader()->setFont(m_main_window->tableFont());*/
		//(*it)->m_widget->verticalHeader()->setDefaultSectionSize(m_main_window->tableRowSize());
		(*it)->verticalHeader()->setDefaultSectionSize(16);
		(*it)->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
		(*it)->setSelectionBehavior(QAbstractItemView::SelectRows);
		(*it)->setSelectionMode(QAbstractItemView::SingleSelection);
		QObject::connect((*it)->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), *it, SLOT(onSectionResized(int, int, int)));

		if (getClosestFeatureState(e_data_table) == e_FtrEnabled && (*it)->config().m_show)
		{
			//(*it)->show();
		}
		else
		{
			//(*it)->hide();
		}
	}
	return it;
}

//@TODO: old call!
#if 0
void Connection::requestTableSynchronization (int sync_group, unsigned long long time)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		if ((*it)->config().m_sync_group == sync_group)
			(*it)->findNearestTimeRow(time);
	}
}

void Connection::requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		if ((*it)->config().m_sync_group == sync_group)
			(*it)->requestTableWheelEventSync(ev, source);

		/*int const hmin = (*it)->horizontalScrollBar()->minimum();
		int const hval = (*it)->horizontalScrollBar()->value();
		int const hmax = (*it)->horizontalScrollBar()->maximum();
		int const vmin = (*it)->verticalScrollBar()->minimum();
		int const vval = (*it)->verticalScrollBar()->value();
		int const vmax = (*it)->verticalScrollBar()->maximum();
		qDebug("conn wh sync: min=%i val=%i max=%i", vmin, vval, vmax);*/
	}
}

void Connection::requestTableActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		if ((*it)->config().m_sync_group == sync_group)
			(*it)->requestTableActionSync(t, cursorAction, modifiers, source);
	}
}
#endif

