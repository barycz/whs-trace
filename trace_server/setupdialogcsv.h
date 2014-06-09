#pragma once
#include <QDialog>
#include <QComboBox>

namespace Ui { class SetupDialogCSV; }

enum E_ImportAction {
	e_Action_Import = 0,
	e_Action_Skip
};

class SetupDialogCSV : public QDialog
{
	Q_OBJECT

public:
	explicit SetupDialogCSV(QWidget *parent = 0);
	~SetupDialogCSV();

	void clear ()
	{
		m_data.clear();
		m_column_actions.clear();
	}

	Ui::SetupDialogCSV * ui;

	QStringList m_data;
	std::vector<int> 	m_column_actions;
};

inline QString getSeparator (QComboBox * box)
{
	if (!box->isEnabled())
		return QString();

	QString separator;
	QString const sep_str = box->currentText();
	if (sep_str == QString("Comma"))          separator = ",";
	else if (sep_str == QString("Tab"))       separator = "\t";
	else if (sep_str == QString("Semicolon")) separator = ";";
	else if (sep_str == QString("Pipe"))      separator = "|";
	else
		separator = sep_str;
	return separator;
}


