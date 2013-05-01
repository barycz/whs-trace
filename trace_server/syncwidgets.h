#pragma once
#include <QObject>

struct SyncWidgets : QObject
{
	SyncWidgets::SyncWidgets (QObject * parent = 0)
		: QObject(parent)
		, m_terminate(false)
	{
		//connect(this, SIGNAL(incomingProfilerConnection(profiler::profiler_rvp_t *)), m_main_window.getServer(), SLOT(incomingProfilerConnection(profiler::profiler_rvp_t *)), Qt::QueuedConnection);
		//printf("+++ connection\n");
	}

	void terminate () { m_terminate = true; }

signals:
	void requestTimeSynchronization (int sync_group, unsigned long long time, void * source);
	void requestFrameSynchronization (int sync_group, unsigned long long frame, void * source);

public slots:
	void performTimeSynchronization (int sync_group, unsigned long long time, void * source)
	{
		emit requestTimeSynchronization(sync_group, time, source);
	}

	void performFrameSynchronization (int sync_group, unsigned long long frame, void * source)
	{
		emit requestFrameSynchronization(sync_group, frame, source);
	}

private:
	bool m_terminate;
	Q_OBJECT
};

extern SyncWidgets g_syncWidgets;

inline SyncWidgets & getSyncWidgets () { return g_syncWidgets; }

