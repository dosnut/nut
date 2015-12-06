//
// C++ Interface: qobject_timer
//
// Description:
//   simple wrapper to manage timer ids in a QObject
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2015
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _NUTS_QOBJECT_TIMER_H
#define _NUTS_QOBJECT_TIMER_H

#include <QObject>

namespace nuts {
	class QObjectTimer {
	public:
		bool match(QTimerEvent *event) const;
		bool active() const;

		void set_timeout(QObject* parent, int msec);
		void kill(QObject* parent);

	private:
		int m_timer_id{-1};
	};
}

#endif
