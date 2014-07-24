#pragma once
#include <QObject>
#include <QWidget>
#include <QIcon>
#include <QToolButton>
#include "cmd.h"

class Connection;

enum E_FilterType {
	e_Filter_Mgr = 0,
	e_Filter_Script,
	e_Filter_String,
	e_Filter_Regex,
	e_Filter_Ctx,
	e_Filter_Lvl,
	e_Filter_Tid,
	e_Filter_FileLine,
	e_Filter_Row,
	e_Filter_Time,
	//e_Filter_Function,
	//e_Filter_dt,
	e_Colorizer_Mgr,
	e_Colorizer_Script,
	e_Colorizer_String,
	e_Colorizer_Regex,
	e_Colorizer_Ctx,
	e_Colorizer_Lvl,
	e_Colorizer_Tid,
	e_Colorizer_FileLine,
	e_Colorizer_Row,
	e_Colorizer_Time,
	//e_Colorizer_Function,
	//e_Colorizer_dt,

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
	QString("Row"),
	QString("Time"),
	//QString("Fn"),
	//QString("dt"),
	QString("Col_Mgr"),
	QString("Col_Script"),
	QString("Col_String"),
	QString("Col_Regex"),
	QString("Col_Ctx"),
	QString("Col_Level"),
	QString("Col_Tid"),
	QString("Col_FileLn"),
	QString("Col_Row"),
	QString("Col_Time"),
	//QString("Col_Fn"),
	//QString("Col_dt"),
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

class FilterBase : public QWidget
{
	Q_OBJECT
public:
	FilterBase (QWidget * parent = 0);
	virtual ~FilterBase ();

	virtual void initUI () = 0;
	virtual void doneUI () = 0;
	QWidget const * ui () const { return m_widget; }
	QWidget * ui () { return m_widget; }

	virtual E_FilterType type () const = 0;
	QString typeName () const { return g_filterNames[this->type()]; }

	virtual bool accept (DecodedCommand const & cmd) const = 0;
	virtual bool action (DecodedCommand const & cmd) { return true; }
	virtual bool enabled () const { return m_enabled; }
	virtual void enable (bool state) { m_enabled = state; }
	virtual void emitFilterChangedSignal ();

	virtual void defaultConfig () = 0;
	virtual void loadConfig (QString const & path) = 0;
	virtual void saveConfig (QString const & path) = 0;
	virtual void applyConfig ();

	virtual void clear () = 0;

	void setConnection(Connection * connection);
	Connection * connection() { return m_connection; }
	const Connection * connection() const { return m_connection; }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("enabled", m_enabled);
	}

public slots:
	void onTabButton ();

signals:
	void filterEnabledChanged ();
	void filterChangedSignal ();
	void connectionChanged(Connection * newConnection);

private slots:
	void onConnectionDestroyed(QObject * conn);

private:
	bool m_enabled;
	QWidget * m_widget;
	QToolButton * m_button;
	Connection * m_connection;
};
