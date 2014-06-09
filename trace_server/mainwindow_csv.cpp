#include "mainwindow.h"
#include "serialize.h"
#include "connection.h"
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include "setupdialogcsv.h"
#include "ui_setupdialogcsv.h"
#include "utils.h"

// log csv file
void MainWindow::createTailLogStream (QString const & fname, QString const & separator)
{
	Connection * connection = createNewConnection();
	connection->setTailFile(fname);
	if (0 == connection->m_file_csv_stream)
		return;
	QFileInfo fi(fname);
	QString const tag = fi.fileName();
	
	connection->handleCSVSetup(tag);
	datalogs_t::iterator it = connection->findOrCreateLog(tag);
	//connection->m_config.m_csv_separator = separator;
	(*it)->m_config.m_csv_separator = separator;
	(*it)->m_config.m_csv_has_header = false;
	(*it)->m_config.m_simplify_strings = false;
	(*it)->m_config.m_unquote_strings = false;
	(*it)->m_config.m_auto_scroll = true;

	// the order is imposed in main.cpp, in the Qt log redirector
	(*it)->m_config.m_storage_order.clear();
	(*it)->m_config.m_storage_order.push_back("Col0");
	(*it)->m_config.m_storage_order.push_back("Lvl");
	(*it)->m_config.m_storage_order.push_back("TID");
	(*it)->m_config.m_storage_order.push_back("Msg");

	if (bool const has_no_setup = (*it)->config().m_columns_setup.size() == 0)
	{
		int const sizes[4] = { 128, 16, 32, 640 };
		E_Align const aligns[4] = { e_AlignRight, e_AlignRight, e_AlignRight, e_AlignLeft };
		E_Elide const elides[4] = { e_ElideLeft, e_ElideLeft, e_ElideLeft, e_ElideRight };

		for (size_t cl = 0, cle = (*it)->m_config.m_storage_order.size(); cl < cle; ++cl)
		{
			QString const & val = (*it)->m_config.m_storage_order[cl];
			int const size = sizes[cl];
			E_Align const align = aligns[cl];
			E_Elide const elide = elides[cl];
			(*it)->config().m_columns_setup.push_back(val);
			(*it)->config().m_columns_sizes.push_back(size);
			(*it)->config().m_columns_align.push_back(QString(alignToString(align)));
			(*it)->config().m_columns_elide.push_back(QString(elideToString(elide)));
		}
	}

	connection->processTailCSVStream();
	emit newConnection(connection);
}

// csv or txt file
void MainWindow::createTailDataStream (QString const & fname)
{
	QString const tag = QFileInfo(fname).fileName();

	Connection * connection = createNewConnection();
	connection->handleCSVSetup(tag);

	logs::LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	cfg.m_tag = tag;
	if (loaded)
	{
		connection->setTailFile(fname);
		datalogs_t::iterator it = connection->findOrCreateLog(tag);

		mentionStringInRecentHistory_Ref(fname, m_config.m_recent_history);
	}
	else
	{
		bool const file_ready = executeSetupDialogCSV(fname);
		if (!file_ready)
		{
			onCloseConnection(connection); // deletes it immeadiately
			return;
		}

		connection->setTailFile(fname);
		datalogs_t::iterator it = connection->findOrCreateLog(tag);

		mentionStringInRecentHistory_Ref(fname, m_config.m_recent_history);

		bool const enable_unquote = m_setup_dialog_csv->ui->unquoteCheckBox->isChecked();
		bool const enable_simplify = m_setup_dialog_csv->ui->simplifyCheckBox->isChecked();
		bool const has_sep = m_setup_dialog_csv->ui->separatorCheckBox->isChecked();
		bool const has_hdr = m_setup_dialog_csv->ui->headerCheckBox->isChecked();
		if (has_sep)
		{
			QString const separator = getSeparator(m_setup_dialog_csv->ui->separatorComboBox);
			//connection->m_config.m_csv_separator = separator;
			(*it)->m_config.m_csv_separator = separator;
		}
		(*it)->m_config.m_csv_has_header = has_hdr;
		(*it)->m_config.m_simplify_strings = enable_simplify;
		(*it)->m_config.m_unquote_strings = enable_unquote;

		if (0 == (*it)->m_config.m_storage_order.size())
		{
			QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
			int const rows = c_model->rowCount();
			(*it)->m_config.m_storage_order.resize(rows);
			for (int i = 0 ; i < rows ; ++i)
			{
				QString const val = c_model->index(i, 0).data(Qt::DisplayRole).toString();
				(*it)->m_config.m_storage_order[i] = val;
			}
		}

		if (bool const has_no_setup = (*it)->config().m_columns_setup.size() == 0)
		{
			QStandardItemModel * p_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->preView->model());
			QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
			for (int cl = 0, cle = c_model->rowCount(); cl < cle; ++cl)
			{
				if (e_Action_Import == m_setup_dialog_csv->m_column_actions[cl])
				{
					QString const c_val = c_model->index(cl, 0).data(Qt::DisplayRole).toString();
					int size = 128;
					for (int pc = 0, pce = p_model->columnCount(); pc < pce; ++pc)
					{
						QString const p_val = p_model->headerData(pc, Qt::Horizontal).toString();
						if (p_val == c_val)
							size = m_setup_dialog_csv->ui->preView->horizontalHeader()->sectionSize(pc);
					}
					E_Align const align = e_AlignLeft;
					E_Elide const elide = e_ElideRight;
					(*it)->config().m_columns_setup.push_back(c_val);
					(*it)->config().m_columns_sizes.push_back(size);
					(*it)->config().m_columns_align.push_back(QString(alignToString(align)));
					(*it)->config().m_columns_elide.push_back(QString(elideToString(elide)));
				}
			}
		}
	}

	connection->processTailCSVStream();
	emit newConnection(connection);
}


void MainWindow::onChangeSeparatorDialogCSV (int n)
{
	if (n == Qt::Unchecked)
	{
		m_setup_dialog_csv->ui->separatorComboBox->setEnabled(false);
		m_setup_dialog_csv->ui->columnList->setEnabled(false);
	}
	else
	{
		m_setup_dialog_csv->ui->separatorComboBox->setEnabled(true);
		m_setup_dialog_csv->ui->columnList->setEnabled(true);
	}
	onChangeSetupDialogCSV(0);
}

void MainWindow::onChangeColumnReset ()
{
	onChangeSetupDialogCSV(0);
}

void MainWindow::onChangeColumnSkipAll ()
{
	QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
	int const rows = c_model->rowCount();
	for (int i = 0 ; i < rows ; ++i)
		m_setup_dialog_csv->m_column_actions[i] = e_Action_Skip;
	onChangeColumnImport();
}

void MainWindow::onChangeColumnImport ()
{
	QStandardItemModel * p_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->preView->model());
	p_model->clear();

	bool const has_sep = m_setup_dialog_csv->ui->separatorCheckBox->isChecked();
	bool const enable_unquote = m_setup_dialog_csv->ui->unquoteCheckBox->isChecked();
	bool const enable_simplify = m_setup_dialog_csv->ui->simplifyCheckBox->isChecked();
	if (has_sep)
	{
		QString separator = getSeparator(m_setup_dialog_csv->ui->separatorComboBox);

		bool const has_hdr = m_setup_dialog_csv->ui->headerCheckBox->isChecked();
		int start = 0;
		if (has_hdr)
		{
			start = 1;
			QStringList hdr_src = m_setup_dialog_csv->m_data.at(0).split(separator);
			QStringList hdr;
			for (int cl = 0, cle = hdr_src.size(); cl < cle; ++cl)
				if (e_Action_Import == m_setup_dialog_csv->m_column_actions[cl])
					hdr << unquoteString(hdr_src.at(cl), enable_unquote, enable_simplify);

			p_model->setHorizontalHeaderLabels(hdr);

			for (int cl = 0, cle = hdr.size(); cl < cle; ++cl)
				m_setup_dialog_csv->ui->preView->horizontalHeader()->setSectionResizeMode(cl, QHeaderView::ResizeToContents);

		}

		for (int i = start, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
		{
			QStringList di = m_setup_dialog_csv->m_data.at(i).split(separator);
			QList<QStandardItem *> l;
			for (int cl = 0, cle = di.size(); cl < cle; ++cl)
				if (e_Action_Import == m_setup_dialog_csv->m_column_actions[cl])
					l << new QStandardItem(unquoteString(di.at(cl), enable_unquote, enable_simplify));
			p_model->appendRow(l);
		}

		//m_setup_dialog_csv->ui->preView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	}
	else
	{
		for (int i = 0, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
			p_model->appendRow(new QStandardItem(unquoteString(m_setup_dialog_csv->m_data.at(i), enable_unquote, enable_simplify)));
	}
}

QString get_user_tag_name (size_t i) { return QString("Col") + QString::number(i); }

void MainWindow::onChangeSetupDialogCSV (int n)
{
	QStandardItemModel * p_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->preView->model());
	QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
	p_model->clear();
	c_model->clear();
	m_setup_dialog_csv->ui->actionGroupBox->setEnabled(false);
	m_setup_dialog_csv->m_column_actions.clear();

	bool const has_sep = m_setup_dialog_csv->ui->separatorCheckBox->isChecked();
	QString const separator = getSeparator(m_setup_dialog_csv->ui->separatorComboBox);
	bool const enable_unquote = m_setup_dialog_csv->ui->unquoteCheckBox->isChecked();
	bool const enable_simplify = m_setup_dialog_csv->ui->simplifyCheckBox->isChecked();
	bool const has_hdr = m_setup_dialog_csv->ui->headerCheckBox->isChecked();

	QStringList hdr;
	int const start = has_hdr ? 1 : 0;
	if (has_sep)
	{

		QStringList hdr_src = m_setup_dialog_csv->m_data.at(0).split(separator);
		if (has_hdr)
		{
			for (int cl = 0, cle = hdr_src.size(); cl < cle; ++cl)
				hdr << unquoteString(hdr_src.at(cl), enable_unquote, enable_simplify);
		}
		else
		{
			for (int cl = 0, cle = hdr_src.size(); cl < cle; ++cl)
				hdr << get_user_tag_name(cl);
		}
	}
	else
	{
		if (has_hdr)
		{
			hdr << unquoteString(m_setup_dialog_csv->m_data.at(0), enable_unquote, enable_simplify);
		}
		else
		{
			hdr << get_user_tag_name(0); // treat file as 1 column
		}
	}

	p_model->setHorizontalHeaderLabels(hdr);
	for (int cl = 0, cle = hdr.size(); cl < cle; ++cl)
	{
		//m_setup_dialog_csv->ui->preView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_setup_dialog_csv->ui->preView->horizontalHeader()->setSectionResizeMode(cl, QHeaderView::ResizeToContents);
		c_model->appendRow(new QStandardItem(unquoteString(hdr.at(cl), enable_unquote, enable_simplify)));
		m_setup_dialog_csv->m_column_actions.push_back(e_Action_Import);
	}

	for (int i = start, ie = m_setup_dialog_csv->m_data.size(); i < ie; ++i)
	{
		if (has_sep)
		{
			QStringList di = m_setup_dialog_csv->m_data.at(i).split(separator);
			QList<QStandardItem *> l;
			for (int col = 0, cole = di.size(); col < cole; ++col)
				l << new QStandardItem(unquoteString(di.at(col), enable_unquote, enable_simplify));
			p_model->appendRow(l);
		}
		else
		{
			p_model->appendRow(new QStandardItem(unquoteString(m_setup_dialog_csv->m_data.at(i), enable_unquote, enable_simplify)));
		}
	}
}

void MainWindow::onChangeColumnRadioDialogCSV (bool toggled)
{
	QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
	QModelIndex const idx = m_setup_dialog_csv->ui->columnList->currentIndex();
	if (idx.isValid())
	{
		int const row = idx.row();
		bool const on = m_setup_dialog_csv->ui->importButton->isChecked();
		m_setup_dialog_csv->m_column_actions[row] = (on) ? e_Action_Import : e_Action_Skip;

		onChangeColumnImport();
	}
}

void MainWindow::onChangeColumnDialogCSV (QModelIndex const & idx)
{
	if (idx.isValid())
	{
		m_setup_dialog_csv->ui->actionGroupBox->setEnabled(true);
		QStandardItemModel * c_model = static_cast<QStandardItemModel *>(m_setup_dialog_csv->ui->columnList->model());
		int const row = idx.row();
		int const action = m_setup_dialog_csv->m_column_actions.at(row);
		m_setup_dialog_csv->ui->importButton->blockSignals(true);
		m_setup_dialog_csv->ui->skipButton->blockSignals(true);
		if (action == e_Action_Import)
			m_setup_dialog_csv->ui->importButton->setChecked(true);
		if (action == e_Action_Skip)
			m_setup_dialog_csv->ui->skipButton->setChecked(true);
		m_setup_dialog_csv->ui->importButton->blockSignals(false);
		m_setup_dialog_csv->ui->skipButton->blockSignals(false);
	}
}

void MainWindow::syncHistoryToRecent (History<QString> const & h)
{
	for (int i = 0; i < m_recent_files.size(); ++i)
	{
		disconnect(m_recent_files[i], SIGNAL(triggered()), this, SLOT(onRecentFile()));
		m_file_menu->removeAction(m_recent_files[i]);
		delete m_recent_files[i];
		m_recent_files[i] = 0;
	}

	m_recent_files.clear();
	for (int i = 0; i < m_config.m_recent_history.size(); ++i)
	{
		m_recent_files.push_back(new QAction(this));
		m_recent_files.back()->setVisible(true);
		QString const fname = QFileInfo(h[i]).fileName();
		m_recent_files.back()->setText(tr("&%1 %2").arg(i + 1).arg(fname));
		m_recent_files.back()->setData(h[i]);
		connect(m_recent_files.back(), SIGNAL(triggered()), this, SLOT(onRecentFile()));
		m_file_menu->insertAction(m_before_action, m_recent_files.back());
	}
}

void MainWindow::mentionStringInRecentHistory_NoRef (QString const & str, History<QString> & h)
{
	if (str.isEmpty())
		return;
	h.insert_no_refcount(str);
	syncHistoryToRecent(h);
}

void MainWindow::mentionStringInRecentHistory_Ref (QString const & str, History<QString> & h)
{
	if (str.isEmpty())
		return;
	h.insert(str);
	syncHistoryToRecent(h);
	m_config.saveHistory(m_appdir);
}

void MainWindow::removeStringFromRecentHistory (QString const & str, History<QString> & h)
{
	if (str.isEmpty())
		return;

	h.remove(str);
	syncHistoryToRecent(h);
	m_config.saveHistory(m_appdir);
}

bool MainWindow::executeSetupDialogCSV (QString const & fname)
{
	m_setup_dialog_csv->clear();

	QFile * f = new QFile(fname);
	if (!f->open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(0, tr("Error"), tr("Could not open file\n%1").arg(fname));
		delete f;
		return false;
	}
	QFileInfo fi(fname);
	QString const ext = fi.suffix();
	bool const is_csv = (0 == QString::compare(ext, g_traceFileExtCSV, Qt::CaseInsensitive));
	QTextStream * file_csv_stream = new QTextStream(f);

	enum { e_min_lines = 3, e_max_read = 32 };
	QStringList lines;
	int n_lines = 0, n_read = 0;
	while (!file_csv_stream->atEnd() && n_lines < e_min_lines && n_read < e_max_read)
	{
		QString const line = file_csv_stream->readLine(2048);
		if (!line.isEmpty())
		{
			lines << line;
			++n_lines;
		}
		++n_read;
	}
	delete file_csv_stream;

	m_setup_dialog_csv->m_data = lines; // sample lines from head of the file

	if (!is_csv)
	{
		m_setup_dialog_csv->ui->headerCheckBox->setChecked(false);
		m_setup_dialog_csv->ui->separatorCheckBox->setChecked(false);
		m_setup_dialog_csv->ui->separatorComboBox->setEnabled(false);
	}
	connect(m_setup_dialog_csv->ui->simplifyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->unquoteCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->separatorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSeparatorDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->separatorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	//connect(m_setup_dialog_csv->ui->separatorComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->columnList, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onChangeColumnDialogCSV(QModelIndex const &)));
	connect(m_setup_dialog_csv->ui->importButton, SIGNAL(toggled(bool)), this, SLOT(onChangeColumnRadioDialogCSV(bool)));
	connect(m_setup_dialog_csv->ui->resetButton, SIGNAL(clicked()), this, SLOT(onChangeColumnReset()));
	connect(m_setup_dialog_csv->ui->skipAllButton, SIGNAL(clicked()), this, SLOT(onChangeColumnSkipAll()));

	onChangeSetupDialogCSV(0);
	m_setup_dialog_csv->exec();

	disconnect(m_setup_dialog_csv->ui->simplifyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->unquoteCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->separatorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSeparatorDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->separatorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	//disconnect(m_setup_dialog_csv->ui->separatorComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->headerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	disconnect(m_setup_dialog_csv->ui->columnList, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onChangeColumnDialogCSV(QModelIndex const &)));
	disconnect(m_setup_dialog_csv->ui->importButton, SIGNAL(toggled(bool)), this, SLOT(onChangeColumnRadioDialogCSV(bool)));
	disconnect(m_setup_dialog_csv->ui->skipButton, SIGNAL(toggled(bool)), this, SLOT(onChangeColumnRadioDialogCSV(bool)));
	disconnect(m_setup_dialog_csv->ui->resetButton, SIGNAL(clicked()), this, SLOT(onChangeColumnReset()));
	disconnect(m_setup_dialog_csv->ui->skipAllButton, SIGNAL(clicked()), this, SLOT(onChangeColumnSkipAll()));

	if (m_setup_dialog_csv->result() != QDialog::Accepted)
		return false;
	return true;
}


