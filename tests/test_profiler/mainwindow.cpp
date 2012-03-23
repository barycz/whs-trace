#include "mainwindow.h"
#include "view.h"
#include "bar.h"
#include "blockinfo.h"

#include <QtGui>

MainWindow::MainWindow(std::vector<ProfileInfo> & pis, QWidget * parent)
	: m_profileInfos(pis)
	, QWidget(parent)
{
	populateScene();

	QSplitter * vSplitter = new QSplitter;
	vSplitter->setOrientation(Qt::Vertical);

	View * view = new View("View 0");
	view->view()->setScene(m_scene);
	vSplitter->addWidget(view);

	view = new View("View 1");
	view->view()->setScene(m_scene);
	vSplitter->addWidget(view);

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget(vSplitter);
	setLayout(layout);

	setWindowTitle(tr("Profiler Demo"));
}

	struct HSV { float h; float s; float v; };
	inline float tmp_randf () { return (float)rand()/(float)RAND_MAX; }

void MainWindow::populateScene()
{
	printf("%s\n", __FUNCTION__);
	m_scene = new QGraphicsScene;

	for (size_t p = 0, pe = m_profileInfos.size(); p < pe; ++p)
	{
		ProfileInfo & pi = m_profileInfos[p];

		//printf("p=%u\n", p); fflush(stdout);

		typedef std::map<std::string, QColor> colormap_t;
		colormap_t colors;

		for (size_t f = 0, fe = pi.m_completed_frame_infos.size(); f < fe; ++f)
		{
			threadinfos_t & tis = pi.m_completed_frame_infos[f];
			for (size_t t = 0, te = tis.size(); t < te; ++t)
			{
				blockinfos_t & bis = tis[t];
				for (size_t b = 0, be = bis.size(); b < be; ++b)
				{
					BlockInfo & block = bis[b];
					block.m_tag = block.m_msg;
					size_t const l = block.m_tag.find('[');
					size_t const r = block.m_tag.find(']');
					if (l != std::string::npos && r != std::string::npos)
						block.m_tag.erase(l, r - l);
					colors[block.m_tag] = Qt::gray;
				}
			}
		}

		//printf("color size: %u\n", colors.size()); fflush(stdout);
		if (colors.size() == 0)
			break;

		// pick colors for unique clusters
		std::vector<HSV> ucolors;
		ucolors.reserve(colors.size());
		for (size_t hi = 0; hi < 360; hi += 360 / colors.size())
		{
			HSV hsv;
			hsv.h = hi / 360.0f;
			hsv.s = 0.30f + tmp_randf() * 0.2f - 0.05f;
			hsv.v = 0.85f + tmp_randf() * 0.2f - 0.05f;
			ucolors.push_back(hsv);
		}

		for (size_t f = 0, fe = pi.m_completed_frame_infos.size(); f < fe; ++f)
		{
			threadinfos_t & tis = pi.m_completed_frame_infos[f];

			for (size_t t = 0, te = tis.size(); t < te; ++t)
			{
				blockinfos_t & bis = tis[t];

				for (size_t b = 0, be = bis.size(); b < be; ++b)
				{
					BlockInfo & block = bis[b];

					int w = block.m_delta_t;
					int h = 28;
					qreal x = block.m_time_bgn;
					qreal y = (t + 2) * block.m_layer * 40;
					//printf("f=%2u t=%2u b=%2u    x=%6.1f y=%6.1f w=%4i h=%4i\n", f, t, b, x, y, w, h); fflush(stdout);

					QColor color = Qt::white;
					colormap_t::iterator it = colors.find(block.m_tag);
					if (it != colors.end())
					{
						HSV hsv = ucolors[std::distance(colors.begin(), it)];
						color.setHsvF(hsv.h, hsv.s, hsv.v);
					}

					QGraphicsItem * item = new Bar(block, color, 0, 0, w, h);
					item->setPos(QPointF(x, y));
					m_scene->addItem(item);
					item->setToolTip(QString("frame=%1 thread=%2 %3 [%4 ms]").arg(f).arg(t).arg(block.m_msg.c_str()).arg(block.m_delta_t));
				}
			}
		}

		for (size_t f = 0, fe = pi.m_frames.size(); f < fe; ++f)
		{
			QGraphicsTextItem * txt = new QGraphicsTextItem(QString("frame=%1").arg(f));
			txt->setPos(pi.m_frames[f].first, 10);
			m_scene->addItem(txt);

			QGraphicsLineItem * line1 = new QGraphicsLineItem(pi.m_frames[f].first, 0, pi.m_frames[f].first, 400);
			m_scene->addItem(line1);

			QGraphicsLineItem * line2 = new QGraphicsLineItem(pi.m_frames[f].second, 0, pi.m_frames[f].second, 400);
			m_scene->addItem(line2);
		}
	}
}