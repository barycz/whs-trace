#ifndef CONTROLBARDOCKEDWIDGETS_H
#define CONTROLBARDOCKEDWIDGETS_H

#include <QWidget>

namespace Ui {
class ControlBarDockedWidgets;
}

class ControlBarDockedWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarDockedWidgets(QWidget *parent = 0);
    ~ControlBarDockedWidgets();

private:
    Ui::ControlBarDockedWidgets *ui;
};

#endif // CONTROLBARDOCKEDWIDGETS_H
