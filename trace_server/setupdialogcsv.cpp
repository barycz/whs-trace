#include "setupdialogcsv.h"
#include "ui_setupdialogcsv.h"

SetupDialogCSV::SetupDialogCSV (QWidget * parent)
	: QDialog(parent)
	, ui(new Ui::SetupDialogCSV)
{
	ui->setupUi(this);
}

SetupDialogCSV::~SetupDialogCSV()
{
	delete ui;
}
