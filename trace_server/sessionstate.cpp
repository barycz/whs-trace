#include "sessionstate.h"
#include <tlv_parser/tlv_encoder.h>

SessionState::SessionState (QObject * parent)
	: m_app_idx(-1)
	, m_tab_idx(-2)
	, m_tab_widget(0)
	, m_exclude_content_to_row(0)
	, m_toggle_ref_row(0)
	, m_filter_mode(e_Exclude)
	, m_columns_setup_current(0)
	, m_columns_setup_template(0)
	, m_columns_sizes(0)
	, m_name()
{
	m_colorized_texts.push_back(ColorizedText(".*[Ww]arning.*", QColor(Qt::yellow), e_Bg));
	m_colorized_texts.push_back(ColorizedText(".*[Ee]rror.*", QColor(Qt::red), e_Fg));
}

SessionState::~SessionState ()
{
	qDebug("~SessionState()");
	if (m_columns_setup_current)
		delete m_columns_setup_current;
}

void SessionState::setupColumns (QList<QString> const * cs_template, columns_sizes_t * sizes)
{
	m_columns_sizes = sizes;
	m_columns_setup_template = cs_template;

	if (!m_columns_setup_current)
	{
		m_columns_setup_current = new QList<QString>();
	}
	else
	{
		m_columns_setup_current->clear();
	}

	m_tags2columns.clear();
	*m_columns_setup_current = *m_columns_setup_template;
	for (size_t i = 0, ie = cs_template->size(); i < ie; ++i)
	{
		size_t const tag_idx = tlv::tag_for_name(cs_template->at(i).toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			m_tags2columns.insert(tag_idx, static_cast<int>(i)); // column index is int in Qt toolkit
			//qDebug("SessionState::setupColumns col[%u] tag_idx=%u tag_name=%s", i, tag_idx, cs->at(i).toStdString().c_str());
		}
	}
}

void SessionState::setupThreadColors (QList<QColor> const & tc)
{
	m_thread_colors = tc;
}

int SessionState::findColumn4Tag (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}

int SessionState::findColumn4TagInTemplate (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}

void SessionState::insertColumn4Tag (tlv::tag_t tag, int column_idx)
{
	m_tags2columns.insert(tag, column_idx);
}
void SessionState::insertColumn ()
{
	m_columns_setup_current->push_back(QString());
	if (m_columns_sizes->size() < m_columns_setup_current->size())
		m_columns_sizes->push_back(127);
	qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());
}

int SessionState::insertColumn (tlv::tag_t tag)
{
	insertColumn();
	int const column_index = m_columns_setup_current->size() - 1;
	char const * name = tlv::get_tag_name(tag);
	insertColumn4Tag(tag, column_index);

	if (name)
		m_columns_setup_current->operator[](column_index) = QString::fromStdString(name);
	else
		m_columns_setup_current->operator[](column_index) = QString("???");
	return column_index;
}

void SessionState::flipFilterMode (E_FilterMode mode)
{
	if (m_filter_mode != mode)
	{
		// for m_file_filters invert check
		// for m_tid_filters
		// for ...
		for (ctx_filters_t::iterator it = m_ctx_filters.begin(), ite = m_ctx_filters.end(); it != ite; ++it)
		{
			//it->m_filterMode = invert(mode);
		}

		m_filter_mode = mode;
	}
}

void SessionState::sessionExport (SessionExport & e) const
{
	e.m_file_filters.reserve(32 * 1024);
	m_file_filters.export_filter(e.m_file_filters);
}
void SessionState::sessionImport (SessionExport const & e)
{
	boost::char_separator<char> sep("\n");
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(e.m_file_filters, sep);
	for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
	{
		m_file_filters.append(*it, true);
	}
}
void SessionState::makeInexactCopy (SessionState const & rhs)
{
	SessionExport e;
	rhs.sessionExport(e);
	sessionImport(e);
}

void SessionState::clearFilters ()
{
	m_file_filters.clear();
	m_tid_filters.clear();
	//m_regex_filters.clear();
	m_colorized_texts.clear();
	m_collapse_blocks.clear();
}


///////// file filters
bool SessionState::isFileLineExcluded (fileline_t const & item) const
{
	return m_file_filters.is_excluded(item.first + "/" + item.second);
}
bool SessionState::isFileLinePresent (fileline_t const & item, bool & state) const
{
	return m_file_filters.is_present(item.first + "/" + item.second, state);
}
void SessionState::appendFileFilter (fileline_t const & item)
{
	m_file_filters.append(item.first + "/" + item.second, true);
}
void SessionState::appendFileFilter (std::string const & item)
{
	m_file_filters.append(item, true);
}
void SessionState::removeFileFilter (fileline_t const & item)
{
	m_file_filters.exclude_off(item.first + "/" + item.second);
}

///////// ctx filters
bool SessionState::isCtxExcluded (context_t item) const
{
	return std::find(m_ctx_filters.begin(), m_ctx_filters.end(), item) != m_ctx_filters.end();
}
void SessionState::appendCtxFilter (context_t item)
{
	m_ctx_filters.push_back(item);
}
void SessionState::removeCtxFilter (context_t item)
{
	m_ctx_filters.erase(std::remove(m_ctx_filters.begin(), m_ctx_filters.end(), item), m_ctx_filters.end());
}

///////// tid filters
void SessionState::appendTIDFilter (std::string const & item)
{
	m_tid_filters.push_back(item);
}
void SessionState::removeTIDFilter (std::string const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}
bool SessionState::isTIDExcluded (std::string const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}

///////// collapsed scopes
void SessionState::appendCollapsedBlock (QString tid, int from, int to, QString file, QString line)
{
	m_collapse_blocks.push_back(CollapsedBlock(tid, from, to, file, line));
}
bool SessionState::findCollapsedBlock (QString tid, int from, int to) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid && b.m_from == from && to == b.m_to)
			return true;
	}
	return false;
}
bool SessionState::eraseCollapsedBlock (QString tid, int from, int to)
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid && b.m_from == from && to == b.m_to)
		{
			m_collapse_blocks.removeAt(i);
			return true;
		}
	}
	return false;
}
bool SessionState::isBlockCollapsed (QString tid, int row) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid)
		{
			if (b.m_from < row && row < b.m_to)
				return true;
		}
	}
	return false;
}
bool SessionState::isBlockCollapsedIncl (QString tid, int row) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid)
		{
			if (b.m_from <= row && row <= b.m_to)
				return true;
		}
	}
	return false;
}

///////// color filters
bool SessionState::isMatchedColorizedText (QString str, QColor & color, E_ColorRole & role) const
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText const & ct = m_colorized_texts.at(i);
		if (ct.exactMatch(str))
		{
			color = ct.m_qcolor;
			role = ct.m_role;
			return ct.m_isEnabled;
		}
	}
	return false;
}
void SessionState::setRegexColor (std::string const & s, QColor col)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
		{
			ct.m_qcolor = col;
		}
	}
}
void SessionState::setColorRegexChecked (std::string const & s, bool checked)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
		{
			ct.m_isEnabled = checked;
		}
	}
}
void SessionState::removeFromColorRegexFilters (std::string const & s)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
		{
			m_colorized_texts.removeAt(i);
			return;
		}
	}
}
void SessionState::appendToColorRegexFilters (std::string const & s)
{
	m_colorized_texts.push_back(ColorizedText(s, e_Fg));
}



