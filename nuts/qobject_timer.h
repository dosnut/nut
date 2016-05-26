#ifndef _NUTS_QOBJECT_TIMER_H
#define _NUTS_QOBJECT_TIMER_H

#pragma once

#include <QObject>

namespace nuts {
	// simple wrapper to manage timer ids in a QObject
	class QObjectTimer {
	public:
		bool match(QTimerEvent* event) const;
		bool active() const;

		void set_timeout(QObject* parent, int msec);
		void kill(QObject* parent);

	private:
		int m_timer_id{-1};
	};
}

#endif
