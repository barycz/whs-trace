#include "connection.h"
#include <gantt/frameview.h>
#include <QScrollBar>
#include <QSplitter>
#include <utils.h>
#include <utils_qstandarditem.h>
//#include <delegates.h>
//#include <gantt/ganttview.h>
#include <label.h>
#include <syncwidgets.h>
#include "mainwindow.h"

dataframes_t::iterator Connection::findOrCreateFrame (QString const & tag)
{
	dataframes_t::iterator it = dataWidgetFactory<e_data_frame>(tag);
	return it;
}


	/*dataframes_t::iterator GanttWidget::findOrCreateFrameView (int sync_group)
	{
		QString const tag = QString("%1").arg(sync_group);

		dataframes_t::iterator it = m_connection->dataWidgetFactory<e_data_gantt>(tag);

		m_dataframeviews.insert(sync_group, fv);
			//QModelIndex const item_idx = m_data_model->insertItemWithHint(name, template_config.m_show);
		return it;
	}*/


