//
// C++ Implementation: random
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "random.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace nuts {
	static void randInit() {
		quint32 seed;
		int fd = open("/dev/urandom", O_RDONLY);
		if (fd >= 0 && read(fd, &seed, sizeof(seed)) == sizeof(seed)) {
			srand(seed);
		} else {
			srand(time(0));
		}
		if (fd >= 0)
			close(fd);
	}
	
	inline static void checkRandInit() {
		static bool initialized = false;
		if (!initialized) {
			randInit();
			initialized = true;
		}
	}

	quint32 getRandomUInt32() {
		checkRandInit();
		if (RAND_MAX < (unsigned int) -1)
			return (rand() << 16) ^ rand();
		else
			return rand();
	}
}
