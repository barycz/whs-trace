#pragma once
#include "../sysfn/enum_factory.h"

namespace trace {

#define TRACELEVEL_ENUM(XX)							\
		XX(Fatal,)									\
		XX(Error,)									\
		XX(Warning,)								\
		XX(Info,)									\
		XX(Detail,)									\
		XX(Debug,)									\
		XX(Brutus,)	 /* too much detail (per frame useless info etc)*/ \
		XX(max_trace_level,)

FACT_DECLARE_ENUM(E_TraceLevel,TRACELEVEL_ENUM);
FACT_DECLARE_ENUM_TO_STRING(E_TraceLevel,TRACELEVEL_ENUM);

typedef E_TraceLevel level_t;

#define TRACE_INIT_LVL_DICT_PAIR(name,assign) {#name, e_##name},

struct LvlDictPair {
	char const * first;
	level_t second;
};

inline size_t getLevelDictionnary (LvlDictPair const * & out)
{
	static const LvlDictPair s_dict[] = {
		TRACELEVEL_ENUM(TRACE_INIT_LVL_DICT_PAIR)
	};
	out = s_dict;
	return sizeof(s_dict) / sizeof(*s_dict);
}

}

