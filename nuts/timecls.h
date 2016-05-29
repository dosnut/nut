#ifndef _NUTS_TIMECLS_H
#define _NUTS_TIMECLS_H

#pragma once

#include <QString>

#include <sys/types.h>

namespace nuts {
	namespace internal {
		/* used for duration and "absolute time" (as in duration since epoch) */
		struct time_value final {
			void fix() {
				while (m_usec < 0) { m_sec--; m_usec += 1000000; }
				while (m_usec > 1000000) { m_sec++; m_usec -= 1000000; }
			}

			explicit time_value() = default;
			explicit time_value(time_t sec, suseconds_t usec = 0)
			: m_sec(sec), m_usec(usec) {
				fix();
			}

			static time_value fromMsecs(qint64 msecs);
			static time_value randomMsecs(qint64 min, qint64 max);
			static time_value randomSecs(int min, int max);

			time_value operator+(time_value const& a) const {
				return time_value(m_sec + a.m_sec, m_usec + a.m_usec);
			}
			time_value operator-(time_value const& a) const {
				return time_value(m_sec - a.m_sec, m_usec - a.m_usec);
			}

			bool operator<=(time_value const& a) const {
				return (m_sec < a.m_sec) || (m_sec == a.m_sec && m_usec <= a.m_usec);
			}
			bool operator>=(time_value const& a) const {
				return (a <= *this);
			}
			bool operator<(time_value const& a) const {
				return !(a <= *this);
			}
			bool operator>(time_value const& a) const {
				return !(*this >= a);
			}

			int msecs() const {
				return 1000*m_sec + (m_usec+999) / 1000;
			}

			time_t m_sec = 0;
			suseconds_t m_usec = 0;
		};
	}

	class Duration final {
	public:
		explicit Duration() = default;
		explicit Duration(time_t sec, suseconds_t usec = 0)
		: m_value(sec, usec) {
		}

	private:
		explicit Duration(internal::time_value value)
		: m_value(value) {
		}

	public:
		static Duration randomSecs(int min, int max);
		static Duration randomMsecs(qint64 min, qint64 max);

		Duration operator+(Duration const& a) const {
			return Duration(m_value + a.m_value);
		}
		Duration operator-(Duration const& a) const {
			return Duration(m_value - a.m_value);
		}
		Duration& operator+=(Duration const& a) {
			m_value = m_value + a.m_value;
			return *this;
		}
		Duration& operator-=(Duration const& a) {
			m_value = m_value - a.m_value;
			return *this;
		}

		bool operator<=(Duration const& a) const { return m_value <= a.m_value; }
		bool operator>=(Duration const& a) const { return m_value >= a.m_value; }
		bool operator<(Duration const& a) const { return m_value < a.m_value; }
		bool operator>(Duration const& a) const { return m_value > a.m_value; }

		int msecs() const {
			return m_value.msecs();
		}

		QString toString() const {
			return QString("%1.%2").arg(m_value.m_sec).arg(m_value.m_usec, 6, 10, QLatin1Char('0'));
		}

	private:
		friend class Time;

		internal::time_value m_value;
	};

#if 0 /* Time not used right now */
	class Time final {
	public:
		explicit Time() = default;
		explicit Time(time_t sec, suseconds_t usec = 0)
		: m_value(sec, usec) {
		}

	private:
		explicit Time(internal::time_value value)
		: m_value(value) {
		}

	public:
		static Time current();
		static Time random(int min, int max);
		static Time waitRandom(int min, int max);
		static Time wait(time_t sec = 0, suseconds_t usec = 0);

		Time operator+(Duration const& a) const {
			return Time(m_value + a.m_value);
		}
		Time operator-(Duration const& a) const {
			return Time(m_value - a.m_value);
		}
		Duration operator-(Time const& a) const {
			return Duration(m_value - a.m_value);
		}
		Time& operator+=(Duration const& a) {
			m_value = m_value + a.m_value;
			return *this;
		}
		Time& operator-=(Duration const& a) {
			m_value = m_value - a.m_value;
			return *this;
		}

		bool operator<=(Time const& a) const { return m_value <= a.m_value; }
		bool operator>=(Time const& a) const { return m_value >= a.m_value; }
		bool operator<(Time const& a) const { return m_value < a.m_value; }
		bool operator>(Time const& a) const { return m_value > a.m_value; }

	private:
		internal::time_value m_value;
	};
#endif /* Time not used right now */
}

#endif /* _NUTS_TIMECLS_H */
