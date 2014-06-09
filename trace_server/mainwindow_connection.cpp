#include "mainwindow.h"
#include "connection.h"
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include "setupdialogcsv.h"
#include "ui_setupdialogcsv.h"
#include "utils.h"
#include "serialize.h"

Connection * MainWindow::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		if ((*it)->getAppName() == app_name)
			return (*it);
	return 0;
}

void MainWindow::newConnection (Connection * c)
{
	statusBar()->showMessage(tr("Incomming tcp connection!"));
}

Connection * MainWindow::createNewConnection ()
{
	static int conn_counter = 0;
	QString const tmp_name = QString("new_app_") + QString::number(++conn_counter);
	Connection * connection = new Connection(tmp_name, this);
	m_connections.push_back(connection);
	qDebug("created new connection[%u] for connection @ 0x%08x", m_connections.size(), connection);
	return connection;
}

void MainWindow::onCloseConnection (Connection * c)
{
	qDebug("MainWindow::onCloseConection(Connection *=0x%08x)", c);
	connections_t::iterator it = std::find(m_connections.begin(), m_connections.end(), c);
	if (it != m_connections.end())
	{
		Connection * connection = (*it);
		m_connections.erase(it);
		delete connection;
		connection = 0;
	}
}
void MainWindow::onCloseMarkedConnections ()
{
	qDebug("%s", __FUNCTION__);

	std::vector<Connection *> to_delete;
	to_delete.reserve(m_connections.size());

	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if ((*it)->m_marked_for_close)
			to_delete.push_back(*it);
	}

	while (!to_delete.empty())
	{
		onCloseConnection(to_delete.back());
		to_delete.pop_back();
	}
}
void MainWindow::markConnectionForClose (Connection * conn)
{
	conn->m_marked_for_close = true;
	QTimer::singleShot(0, this, SLOT(onCloseMarkedConnections()));
}

// tlv file
void MainWindow::importDataStream (QString const & fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning(QString(tr("Could not open file %1").arg(fname)).toLatin1());
		return;
	}

	Connection * connection = createNewConnection();
	connection->setImportFile(fname);
	statusBar()->showMessage(tr("Importing file!"));

	QDataStream import_stream(&file);
	connection->processDataStream(import_stream);
	file.close();
}


