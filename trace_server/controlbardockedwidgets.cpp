#include "controlbardockedwidgets.h"
#include "ui_controlbardockedwidgets.h"

ControlBarDockedWidgets::ControlBarDockedWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarDockedWidgets)
{
    ui->setupUi(this);
}

ControlBarDockedWidgets::~ControlBarDockedWidgets()
{
    delete ui;
}
