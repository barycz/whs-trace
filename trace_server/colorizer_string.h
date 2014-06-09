#pragma once
#include "filterbase.h"
#include "ui_colorizer_string.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include <vector>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <logs/logtablemodel.h>

struct ColorizedString {
	bool	m_is_enabled;
	QColor	m_fgcolor;
	QColor	m_bgcolor;
	QString m_str;
	bool	m_case_sensitive;
	bool	m_whole_word;

	bool isValid () const { return !m_str.isEmpty(); }

	bool accept (QString str) const
	{
		Qt::CaseSensitivity const cs = m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
		if (m_is_enabled && str.contains(m_str, cs))
		{
			return true;
		}
		return false;
	}

	ColorizedString () { }
	ColorizedString (QString const & s, QColor const & col, QColor const & bgcol)
        : m_fgcolor(col), m_bgcolor(bgcol), m_str(s), m_is_enabled(0), m_case_sensitive(false), m_whole_word(false)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("fgcolor", m_fgcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
		ar & boost::serialization::make_nvp("str", m_str);
		ar & boost::serialization::make_nvp("case_sensitive", m_case_sensitive);
		ar & boost::serialization::make_nvp("whole_word", m_whole_word);
	}
};


struct ColorizerString : FilterBase
{
	Ui_ColorizerString * m_ui;

	ColorizerString (QWidget * parent = 0);
	virtual ~ColorizerString ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Colorizer_String; }

	virtual bool accept (DecodedCommand const & cmd) const;
	virtual bool action (DecodedCommand const & cmd);

	virtual void defaultConfig ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("colorizer_string", m_data);
	}

	// color_string specific
	ColorizedString & findOrCreateColorizedString (QString const & str);
	void setupModel ();
	void setSrcModel (LogTableModel * m) { m_src_model = m; }
	void destroyModel ();
	ColorizedString const * findMatch (QString const & item) const;
	void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	void setConfigToUI ();

	ColorizedString & add (QString const & str, QColor const & fg, QColor const & bg);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void onColorButtonChanged (int role);

	void actionColor (DecodedCommand const & cmd, ColorizedString const & ct, QColor const & fg, QColor const & bg) const;
	void actionUncolor (DecodedCommand const & cmd, ColorizedString const & ct) const;
	void updateColor (ColorizedString const & ct);
	void uncolor (ColorizedString const & ct);


	typedef std::vector<ColorizedString> filters_t;
	filters_t				m_data;
	QStandardItemModel *	m_model;
	LogTableModel *			m_src_model; // @FIXME: not happy about this, but i need it fast :/

	Q_OBJECT
public slots:
	void onClickedAt (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
	void onAdd ();
	void onRm ();
	void onActivate (int);
	void onFgChanged ();
	void onBgChanged ();

signals:
};

struct ColorizerStringDelegate : public QStyledItemDelegate
{
    ColorizerStringDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
    ~ColorizerStringDelegate ();
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

