#include "timecls.h"

#include "random.h"

#include <sys/time.h>

namespace nuts {
	Time Time::current() {
		struct timeval tv = { 0, 0 };
		gettimeofday(&tv, 0);
		return Time(tv.tv_sec, tv.tv_usec);
	}

	Time Time::random(int min, int max) {
		float range = (max - min);
		float r = min + range * ((float) getRandomUInt32()) / ((float) (quint32) -1);
		int sec = (int) r;
		r -= sec; r *= 1000000;
		int usec = (int) r;
		return Time(sec, usec);
	}

	Time Time::waitRandom(int min, int max) {
		return random(min, max) + current();
	}

	Time Time::wait(time_t sec, suseconds_t usec) {
		return current() + Time(sec, usec);
	}
}
