#pragma once
#include <tlv_parser/tlv.h>
#include <tlv_parser/tlv_cmd_qstring.h>
#include <vector>
#include <QVariant>
#include <QModelIndex>

inline QVariant tag2variant (tlv::tag_t t, QString const & src)
{
	using namespace tlv;
	switch (t)
	{
		case tag_invalid:	return QVariant(src);
		case tag_app:		return QVariant(src);
		case tag_pid:		return QVariant(src);
		case tag_ctime:		return QVariant(src.toLongLong());
		case tag_stime:		return QVariant(src.toLongLong());
		case tag_tid:		return QVariant(src.toLongLong());
		case tag_file:		return QVariant(src);
		case tag_line:		return QVariant(src);
		case tag_func:		return QVariant(src);
		case tag_msg:		return QVariant(src);
		case tag_lvl:		return QVariant(src.toInt());
		case tag_ctx:		return QVariant(src.toInt());
		case tag_bool:		return QVariant(src);
		case tag_int:		return QVariant(src.toInt());
		case tag_string:	return QVariant(src);
		case tag_float:		return QVariant(src.toFloat());
		case tag_x:			return QVariant(src.toDouble());
		case tag_y:			return QVariant(src.toDouble()); 
		case tag_z:			return QVariant(src.toDouble());
		case tag_fgc:		return QVariant(src);
		case tag_bgc:		return QVariant(src);
		case tag_hhdr:		return QVariant(src);
		case tag_vhdr:		return QVariant(src);
		default:			return QVariant(src);
	}
}

struct DecodedCommand : tlv::StringCommand_v1
{
	std::vector<char> m_orig_message;
	bool m_written_hdr;
	bool m_written_payload;
	int m_src_row;
	//void const * m_tree_node;
	int m_row_type;
	int m_indent;

	typedef std::vector<QVariant> tvariants_t;
	tvariants_t m_variants;

	DecodedCommand () : StringCommand_v1() { m_orig_message.resize(2048); reset(); }

	void reset ()
	{
		tlv::StringCommand_v1::reset();
		m_written_hdr = m_written_payload = false;
		memset(&m_orig_message[0], 0, m_orig_message.size());
	}

	template <class T>
	bool get (tlv::tag_t tag, T & t) const
	{
		for (size_t i = 0, ie = m_tvs.size(); i < ie; ++i)
			if (m_tvs[i].m_tag == tag)
			{
				t = m_variants[i].value<T>();
				return true;
			}
		return false;
	}

	bool getString (tlv::tag_t tag, QString & t) const
	{
		for (size_t i = 0, ie = m_tvs.size(); i < ie; ++i)
			if (m_tvs[i].m_tag == tag)
			{
				t = m_tvs[i].m_val;
				return true;
			}
		return false;
	}


	void decode_postprocess ()
	{
		m_variants.reserve(m_tvs.size());
		for (size_t i = 0, ie = m_tvs.size(); i < ie; ++i)
			m_variants.push_back(tag2variant(m_tvs[i].m_tag, m_tvs[i].m_val));
	}
};

