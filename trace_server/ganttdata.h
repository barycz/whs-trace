#pragma once
#include <vector>
#include <string>
#include <cstdio>
#include <QColor>

namespace gantt {

	struct Data
	{
		Data * m_parent;
		unsigned long long m_time_bgn;
		unsigned long long m_time_end;
		unsigned long long m_delta_t;
		unsigned long long m_layer;
		unsigned long long m_ctx;
		unsigned m_ctx_idx;
		unsigned m_frame;
		float m_x;
		float m_y;
		double m_dt;
		QColor m_color;
		QString m_msg;
		QString m_tag;

		Data () 
			: m_parent(0)
			, m_time_bgn(0), m_time_end(0), m_delta_t(0)
			, m_layer(0)
			, m_ctx(0) , m_ctx_idx(0)
			, m_frame(0)
			, m_x(0.0f), m_y(0.0f), m_dt(0.0)
			, m_color(Qt::gray)
		{ }
		
		void complete ()
		{
			m_delta_t = m_time_end - m_time_bgn;
			//m_dt = static_cast<float>(m_delta_t) / g_scaleValue;
			m_dt = static_cast<float>(m_delta_t);
			//printf("completed: tid=%10llu delta_t=%10llu msg=%s\n", m_tid, m_delta_t, m_msg.c_str());
		}
	};

	typedef std::vector<Data *> data_t;

	typedef std::vector<data_t> pendingdata_t;
	typedef std::vector<data_t> contextdata_t;

	typedef std::vector<contextdata_t> framedata_t;
	typedef std::vector<contextdata_t *> ptrframedata_t;

	struct ReceivedData
	{
		ReceivedData () : m_frame(0), m_frame_begin(0) { }

		data_t m_bi_ptrs;

		pendingdata_t m_pending_data;

		std::vector<unsigned long long> m_contexts;
		std::vector<std::pair<float, float> > m_frames;
		std::vector<unsigned> m_critical_paths;
		unsigned m_frame;
		unsigned m_frame_begin;
		ptrframedata_t m_completed_frame_data;
	};
} // namespace gantt

