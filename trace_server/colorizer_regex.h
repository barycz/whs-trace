#pragma once
#include "filterbase.h"
#include "ui_colorizer_regex.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include <vector>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <logs/logtablemodel.h>

struct ColorizedText {
	bool	m_is_enabled;
	QColor	m_fgcolor;
	QColor	m_bgcolor;
	QString m_regex_str;
	QRegExp m_regex;

	bool isValid () const { return m_regex.isValid(); }

	bool accept (QString str) const
	{
		if (m_is_enabled && m_regex.exactMatch(str))
		{
			return true;
		}
		return false;
	}

	ColorizedText () { }

	ColorizedText (QString const & rs, QColor const & col, QColor const & bgcol)
        : m_fgcolor(col), m_bgcolor(bgcol), m_regex_str(rs), m_regex(rs), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("fgcolor", m_fgcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
	}
};


struct ColorizerRegex : FilterBase
{
	Ui_ColorizerRegex * m_ui;

	ColorizerRegex (QWidget * parent = 0);
	virtual ~ColorizerRegex ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Colorizer_Regex; }

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
		ar & boost::serialization::make_nvp("colorizer_regex", m_data);
	}

	// color_regex specific
	ColorizedText & findOrCreateColorizedText (QString const & str);
	void setupModel ();
	void setSrcModel (LogTableModel * m) { m_src_model = m; }
	void destroyModel ();
	ColorizedText const * findMatch (QString const & item) const;
	void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	void setConfigToUI ();

	ColorizedText & add (QString const & regex, QColor const & fg, QColor const & bg);
	//void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void onColorButtonChanged (int role);

	void actionColor (DecodedCommand const & cmd, ColorizedText const & ct, QColor const & fg, QColor const & bg) const;
	void actionUncolor (DecodedCommand const & cmd, ColorizedText const & ct) const;
	void updateColor (ColorizedText const & ct);
	void uncolor (ColorizedText const & ct);
	void recompileColorRegexps ();

	void recompileColorRegex (ColorizedText & ct);



	typedef std::vector<ColorizedText> filters_t;
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

struct ColorizerRegexDelegate : public QStyledItemDelegate
{
	ColorizerRegexDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
	~ColorizerRegexDelegate ();
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

