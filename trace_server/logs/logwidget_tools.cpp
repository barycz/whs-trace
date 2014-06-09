#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QStandardItem>
#include "types.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "mainwindow.h"
#include "connection.h"
#include "movablelistmodel.h"
#include <tlv_parser/tlv_parser.h>

namespace logs {

	bool add (QString const & tlvname, int row, bool checked)
	{
		//insertRow(row, addRow(tlvname, checked));
		//beginInsertRows(QModelIndex(), row, row);
		//insertRow(m_data.count());
		//m_data.insert(row, tlvname);
		//endInsertRows();
		//emit layoutChanged();
		return true;
	}

}


