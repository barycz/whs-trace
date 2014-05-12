#pragma once
#include <QWidget>
#include "../qwt/qwt_plot.h"
#include "../qwt/qwt_plot_curve.h"
#include "plotconfig.h"

class QwtPlotCurve;

namespace plot {

	struct Data {
		std::vector<double> m_data_x;
		std::vector<double> m_data_y;

		Data (size_t ln) { reserve(ln); }

		void clear ()
		{
			m_data_x.clear();
			m_data_y.clear();
		}

		void reserve (size_t const n)
		{
			m_data_x.reserve(n);
			m_data_y.reserve(n);
		}

		void push_back (double x, double y)
		{
			m_data_x.push_back(x);
			m_data_y.push_back(y);
		}
	};

	struct Curve : ActionAble
	{
		QwtPlotCurve * m_curve;
		Data * m_data;
		CurveConfig & m_config;

		Curve (CurveConfig & curve, QStringList const & path)
			: ActionAble(path)
			, m_curve(0)
			, m_data(0)
			, m_config(curve)
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);
		}

		QwtPlotCurve const * getCurve () const { return m_curve; }
		QwtPlotCurve * getCurve () { return m_curve; }

		QString const & getName () { return m_config.m_tag; }

		virtual QWidget * controlWidget () { return 0; }
		virtual bool handleAction (Action * a, E_ActionHandleType sync)
		{
			switch (a->type())
			{
				case e_Visibility:
				{
					Q_ASSERT(m_args.size() > 0);
					bool const on = a->m_args.at(0).toBool();
					if (m_curve)
						m_curve->setVisible(on);
					return true;
				}
				default:
					return false;
			}
		}
		
		~Curve ()
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);
			if (m_curve) {
				m_curve->detach();
				delete m_curve;
				m_curve = 0;
			}
			if (m_data) {
				delete m_data;
				m_data = 0;
			}
		}
	};
}

