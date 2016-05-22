#ifndef NUTSTIMECLS_H
#define NUTSTIMECLS_H

#pragma once

#include <QString>

#include <sys/types.h>

namespace nuts {
	class Time final {
	private:
		time_t m_sec = 0;
		suseconds_t m_usec = 0;

		void fix() {
			while (m_usec < 0) { m_sec--; m_usec += 1000000; }
			while (m_usec > 1000000) { m_sec++; m_usec -= 1000000; }
		}

	public:
		explicit Time() = default;
		explicit Time(time_t sec, suseconds_t usec = 0)
		: m_sec(sec), m_usec(usec) {
			fix();
		}

		static Time current();
		static Time random(int min, int max);
		static Time waitRandom(int min, int max);
		static Time wait(time_t sec = 0, suseconds_t usec = 0);

		Time operator+(Time const& a) const {
			return Time(m_sec + a.m_sec, m_usec + a.m_usec);
		}
		Time operator-(Time const& a) const {
			return Time(m_sec - a.m_sec, m_usec - a.m_usec);
		}
		Time& operator+=(Time const& a) {
			m_sec += a.m_sec; m_usec += a.m_usec;
			fix();
			return *this;
		}
		Time& operator-=(Time const& a) {
			m_sec -= a.m_sec; m_usec -= a.m_usec;
			fix();
			return *this;
		}

		bool operator<=(Time const& a) const {
			return (m_sec < a.m_sec) || (m_sec == a.m_sec && m_usec <= a.m_usec);
		}
		bool operator>=(Time const& a) const {
			return (a <= *this);
		}
		bool operator<(Time const& a) const {
			return !(a <= *this);
		}
		bool operator>(Time const& a) const {
			return !(*this >= a);
		}

		int msecs() {
			return 1000*m_sec + (m_usec+999) / 1000;
		}

		QString toString() const {
			return QString("%1.%2").arg(m_sec).arg(m_usec, 6, 10, QLatin1Char('0'));
		}
	};
}

#endif
