#pragma once
#include <QObject>
#include <QWidget>
#include <QIcon>
#include <QToolButton>
#include "cmd.h"

enum E_FilterType {
	e_Filter_Mgr = 0,
	e_Filter_Script,
	e_Filter_String,
	e_Filter_Regex,
	e_Filter_Ctx,
	e_Filter_Lvl,
	e_Filter_Tid,
	e_Filter_FileLine,
	e_Filter_User0,
	e_Filter_User1,
	e_Filter_User2,

	e_filtertype_max_value
};

QString const g_filterNames[] = {
	QString("Mgr"),
	QString("Script"),
	QString("String"),
	QString("Regex"),
	QString("Ctx"),
	QString("Level"),
	QString("Tid"),
	QString("FileLn"),
	QString("User0"),
	QString("User1"),
	QString("User2"),
	QString("max")
};

inline E_FilterType filterName2Type (QString const & name)
{
	for (size_t i = 0; i < e_filtertype_max_value; ++i)
		if (name == g_filterNames[i])
			return static_cast<E_FilterType>(i);
	return e_filtertype_max_value;
}

QIcon grabIcon (bool enabled);

struct FilterBase : public QWidget
{
	bool m_enabled;
	QWidget * m_widget;
	QToolButton * m_button;

	FilterBase (QWidget * parent = 0);
	virtual ~FilterBase ();

	virtual void initUI () = 0;
	virtual void doneUI () = 0;
	QWidget const * ui () const { return m_widget; }
	QWidget * ui () { return m_widget; }

	virtual E_FilterType type () const = 0;
	QString typeName () const { return g_filterNames[this->type()]; }

	virtual bool accept (DecodedCommand const & cmd) const = 0;
	virtual bool enabled () const { return m_enabled; }
	virtual void enable (bool state) { m_enabled = state; }
	virtual void emitFilterChangedSignal ();

	virtual void defaultConfig () = 0;
	virtual void loadConfig (QString const & path) = 0;
	virtual void saveConfig (QString const & path) = 0;
	virtual void applyConfig ();

	virtual void clear () = 0;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("enabled", m_enabled);
	}

	Q_OBJECT
public slots:
	void onTabButton ();
signals:
	void filterEnabledChanged ();
	void filterChangedSignal ();
};
