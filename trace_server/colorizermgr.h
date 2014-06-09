#pragma once
#include "filtermgr.h"
//#include "colorizer_script.h"
#include "colorizer_string.h"
#include "colorizer_regex.h"
//#include "colorizer_ctx.h"
//#include "colorizer_lvl.h"
//#include "colorizer_tid.h"
//#include "colorizer_fileline.h"
#include "colorizer_row.h"

struct ColorizerMgr : FilterMgr
{
	ColorizerMgr (QWidget * parent = 0);
	virtual ~ColorizerMgr ();

	virtual E_FilterType type () const { return e_Colorizer_Mgr; }
	virtual void defaultConfig ();

	virtual void initUI ();
	virtual void doneUI ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual bool accept (DecodedCommand const & cmd) const;
	virtual bool action (DecodedCommand const & cmd);
	virtual void clear ();
	virtual FilterBase * filterFactory (E_FilterType t, QWidget * parent);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterMgr::serialize(ar, version);
	}

	//FilterXX *			getFilterXX () { return static_cast<FilterXX *>(m_cache[e_Filter_XX]); }
	//FilterXX const *		getFilterXX () const { return static_cast<FilterXX const *>(m_cache[e_Filter_XX]); }
	ColorizerRow *			getColorizerRow () { return static_cast<ColorizerRow *>(m_cache[e_Colorizer_Row]); }
	ColorizerRow const *		getColorizerRow () const { return static_cast<ColorizerRow const *>(m_cache[e_Colorizer_Row]); }
	//ColorizerScript *		getColorizerScript () { return static_cast<ColorizerScript *>(m_cache[e_Colorizer_Script]); }
	//ColorizerScript const *	getColorizerScript () const { return static_cast<ColorizerScript const *>(m_cache[e_Colorizer_Script]); }
	ColorizerString *			getColorizerString () { return static_cast<ColorizerString *>(m_cache[e_Colorizer_String]); }
	ColorizerString const *	getColorizerString () const { return static_cast<ColorizerString const *>(m_cache[e_Colorizer_String]); }
	ColorizerRegex *			getColorizerRegex () { return static_cast<ColorizerRegex *>(m_cache[e_Colorizer_Regex]); }
	ColorizerRegex const *		getColorizerRegex () const { return static_cast<ColorizerRegex const *>(m_cache[e_Colorizer_Regex]); }
	//ColorizerCtx *				getColorizerCtx () { return static_cast<ColorizerCtx *>(m_cache[e_Colorizer_Ctx]); }
	//ColorizerCtx const *		getColorizerCtx () const { return static_cast<ColorizerCtx const *>(m_cache[e_Colorizer_Ctx]); }
	//ColorizerLvl *				getColorizerLvl () { return static_cast<ColorizerLvl *>(m_cache[e_Colorizer_Lvl]); }
	//ColorizerLvl const *		getColorizerLvl () const { return static_cast<ColorizerLvl const *>(m_cache[e_Colorizer_Lvl]); }
	//ColorizerTid *				getColorizerTid () { return static_cast<ColorizerTid *>(m_cache[e_Colorizer_Tid]); }
	//ColorizerTid const *		getColorizerTid () const { return static_cast<ColorizerTid const *>(m_cache[e_Colorizer_Tid]); }
	//ColorizerFileLine *		getColorizerFileLine () { return static_cast<ColorizerFileLine *>(m_cache[e_Colorizer_FileLine]); }
	//ColorizerFileLine const *	getColorizerFileLine () const { return static_cast<ColorizerFileLine const *>(m_cache[e_Colorizer_FileLine]); }

	//void clearUI ();
	//void setConfigToUI ();
	//void setUIToConfig ();

public slots:
signals:

public:
	Q_OBJECT
};

