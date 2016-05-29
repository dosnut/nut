#include "timecls.h"

#include "random.h"

#include <limits>

// #include <sys/time.h>

namespace nuts {
	namespace internal {
		time_value time_value::fromMsecs(qint64 msecs) {
			time_t secs = static_cast<time_t>(msecs / 1000);
			suseconds_t usec = static_cast<suseconds_t>((msecs % 1000) * 1000);
			return time_value(secs, usec);
		}

		time_value time_value::randomMsecs(qint64 min, qint64 max) {
			std::uniform_int_distribution<qint64> d(min, max);
			return fromMsecs(d(randomGenerator));
		}

		time_value time_value::randomSecs(int min, int max) {
			return randomMsecs(qint64(1000) * min, qint64(1000) * max);
		}
	}

	Duration Duration::randomSecs(int min, int max) {
		return Duration(internal::time_value::randomSecs(min, max));
	}

	Duration Duration::randomMsecs(qint64 min, qint64 max) {
		return Duration(internal::time_value::randomMsecs(min, max));
	}

#if 0
	Time Time::current() {
		struct timeval tv = { 0, 0 };
		gettimeofday(&tv, 0);
		return Time(tv.tv_sec, tv.tv_usec);
	}

	Time Time::waitRandom(int min, int max) {
		return current() + Duration::randomSecs(min, max);
	}

	Time Time::wait(time_t sec, suseconds_t usec) {
		return current() + Duration(sec, usec);
	}
#endif
}
