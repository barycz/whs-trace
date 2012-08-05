#pragma once
#include <QtGui/qwidget.h>
#include "baseplot.h"
#include "curves.h"

class QwtPlotCurve;
class QwtPlotMarker;

namespace plot {

	class PickablePlot : public BasePlot
	{
		Q_OBJECT
	public:

		PickablePlot (QWidget * = 0);
		~PickablePlot ();

	private Q_SLOTS:

	private:
		int m_timer;
		std::vector<QwtPlotMarker *> m_markers;
	};
}
