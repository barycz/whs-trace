#include "colorizewidget.h"
#include "ui_colorizewidget.h"
#include "utils_history.h"
#include "colorizeconfig.h"
#include "mainwindow.h"
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>

void ColorizeWidget::init ()
{
	m_ui->setupUi(this);

	QStyle const * const style = QApplication::style();
	connect(m_ui->findBox, SIGNAL(editTextChanged(QString)), this, SLOT(onEditTextChanged(QString)));

	QLineEdit * le = m_ui->findBox->lineEdit();
	connect(le, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_ui->colorizeButton, SIGNAL(clicked()), this, SLOT(onColorizeString()));
	//connect(m_ui->refsButton, SIGNAL(clicked()), this, SLOT(onColorizeAllRefs()));
	//connect(m_ui->cloneButton, SIGNAL(clicked()), this, SLOT(onColorizeAllClone()));
	connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(m_ui->nextButton, SIGNAL(clicked()), this, SLOT(onColorizeNext()));
	connect(m_ui->prevButton, SIGNAL(clicked()), this, SLOT(onColorizePrev()));

	m_ui->fgButton->setStandardColors();
	m_ui->fgButton->setCurrentColor(QColor(Qt::blue));
	m_ui->bgButton->setStandardColors();
	m_ui->bgButton->setCurrentColor(QColor(Qt::white));

	setAutoFillBackground(true);
}

ColorizeWidget::ColorizeWidget (MainWindow * mw, QWidget * parent)
	: QWidget(parent)
	, m_ui(new Ui::ColorizeWidget)
	, m_main_window(mw)
	, m_aa(0)
{
	hide();
	init();
}

ColorizeWidget::ColorizeWidget (QWidget * parent) // widget coming from Qt creator
	: QWidget(parent)
	, m_ui(new Ui::ColorizeWidget)
	, m_main_window(0)
	, m_aa(0)
{
	hide();
	init();
}

ColorizeWidget::~ColorizeWidget ()
{
	delete m_ui;
}

void ColorizeWidget::applyConfig (ColorizeConfig & cfg)
{
	m_config = cfg;
	setConfigValuesToUI(m_config);
}

void ColorizeWidget::applyConfig ()
{
	applyConfig(m_config);
}

void ColorizeWidget::onCancel ()
{
  QObject * o = parent();
  QWidget * w = qobject_cast<QWidget *>(o);
  w->setFocus();
  hide();
}

void ColorizeWidget::onActivate ()
{
	show();
	activateWindow();
	m_ui->findBox->setFocus();
	raise();
}

void ColorizeWidget::onEditTextChanged (QString str)
{
	onResetRegexpState();
	//qDebug("find!");
}

void ColorizeWidget::onReturnPressed ()
{
	onColorizeNext();
	//m_config.saveHistory();
}

void ColorizeWidget::focusNext ()
{
	QWidget * const curr =  qApp->focusWidget();
	QWidget * const next = curr->nextInFocusChain();
	next->setFocus(Qt::TabFocusReason);
}

void ColorizeWidget::focusPrev ()
{
	QWidget * const curr =  qApp->focusWidget();
	QWidget * const prev = curr->previousInFocusChain();
	prev->setFocus(Qt::TabFocusReason);
}

/*void ColorizeWidget::onFocusChanged (QWidget * old, QWidget * now)
{
	//m_find_widget->onFocusChanged(old, now);
}*/

void ColorizeWidget::onResetRegexpState ()
{
	m_ui->findBox->setStyleSheet("");
	m_ui->findBox->setToolTip("");
}

void ColorizeWidget::signalRegexpState (E_ExprState state, QString const & reason)
{
	if (state == e_ExprInvalid)
	{
		m_ui->findBox->setStyleSheet(
			"QComboBox {\
				 border: 1px solid red;\
				 border-radius: 3px;\
			}");

		m_ui->findBox->setToolTip(reason);
	}
	else
	{
		m_ui->findBox->setStyleSheet(
			"QComboBox {\
				 border: 1px solid green;\
				 border-radius: 3px;\
			}");
		m_ui->findBox->setToolTip("");
	}
}

void ColorizeWidget::makeActionColorize (QString const & str, Action & a)
{
	a.m_type = e_Colorize;
	//a.m_src_path = path();
	//a.m_src = this;
	if (m_aa)
		a.m_dst_path = m_aa->path();
	QVariant fc;
	fc.setValue(m_config);
	a.m_args.push_back(fc);
}

void ColorizeWidget::colorize ()
{
	QString const str = m_ui->findBox->currentText();
	if (!str.isEmpty())
	{
		mentionStringInHistory_Ref(str, m_ui->findBox, m_config.m_history);
		m_ui->findBox->setCurrentIndex(m_ui->findBox->findText(str));
		setUIValuesToConfig(m_config);
		m_config.m_str = str;
		if (m_config.m_regexp)
		{
			m_config.m_regexp_val = QRegExp(m_config.m_str);
			if (m_config.m_regexp_val.isValid())
			{
				signalRegexpState(e_ExprValid, "");
			}
			else
			{
				QString const & reason = m_config.m_regexp_val.errorString();
				signalRegexpState(e_ExprInvalid, QString("regexp failed: ") + reason);
				return;
			}
		}
		Action a;
		makeActionColorize(str, a);
		m_main_window->dockManager().handleAction(&a, e_Sync);
		QTimer::singleShot(750, this, SLOT(onResetRegexpState()));
	}

}

void ColorizeWidget::colorize (bool select, bool refs, bool clone)
{
	m_config.m_next = 0;
	m_config.m_prev = 0;
	m_config.m_select = select;
	m_config.m_refs = refs;
	m_config.m_clone = clone;
	colorize();
}
void ColorizeWidget::onColorizeString () { colorize(1, 0, 0); }
//void ColorizeWidget::onColorizeAllRefs () { colorize(0, 1, 0); }
//void ColorizeWidget::onColorizeAllClone () { colorize(0, 0, 1); }

void ColorizeWidget::colorize (bool prev, bool next)
{
	m_config.m_next = next;
	m_config.m_prev = prev;
	m_config.m_select = 1;
	m_config.m_refs = 0;
	m_config.m_clone = 0;
	colorize();
}
void ColorizeWidget::onColorizeNext () { colorize(0, 1); }
void ColorizeWidget::onColorizePrev () { colorize(1, 0); }

void ColorizeWidget::clearUI ()
{
	m_ui->widgetComboBox->clear();
	m_ui->findBox->clear();
}

void ColorizeWidget::setConfigValuesToUI (ColorizeConfig const & cfg)
{
	clearUI();
	syncHistoryToWidget(m_ui->findBox, cfg.m_history);
	m_ui->caseCheckBox->setChecked(cfg.m_case_sensitive);
	m_ui->wholeWordCheckBox->setChecked(cfg.m_whole_word);
	m_ui->regexCheckBox->setChecked(cfg.m_regexp);
	m_ui->widgetComboBox->addItems(cfg.m_to_widgets);
	m_ui->fgButton->setCurrentColor(cfg.m_fgcolor);
	m_ui->bgButton->setCurrentColor(cfg.m_bgcolor);
}

void ColorizeWidget::setUIValuesToConfig (ColorizeConfig & cfg)
{
	cfg.m_case_sensitive = m_ui->caseCheckBox->isChecked();
	cfg.m_whole_word = m_ui->wholeWordCheckBox->isChecked();
	cfg.m_regexp = m_ui->regexCheckBox->isChecked();
	cfg.m_str = m_ui->findBox->currentText();
	cfg.m_fgcolor = m_ui->fgButton->currentColor();
	cfg.m_bgcolor = m_ui->bgButton->currentColor();
}

