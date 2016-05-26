#include "qobject_timer.h"

#include <QTimerEvent>

namespace nuts {
	bool QObjectTimer::match(QTimerEvent* event) const
	{
		return (event->timerId() == m_timer_id);
	}

	bool QObjectTimer::active() const {
		return -1 != m_timer_id;
	}

	void QObjectTimer::set_timeout(QObject* parent, int msec)
	{
		kill(parent);
		if (msec > 0) {
			m_timer_id = parent->startTimer(msec);
		}
	}

	void QObjectTimer::kill(QObject* parent)
	{
		if (-1 != m_timer_id) {
			parent->killTimer(m_timer_id);
			m_timer_id = -1;
		}
	}
}
