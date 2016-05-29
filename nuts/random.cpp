#include "random.h"

#include <random>

namespace nuts {
	std::mt19937 randomGenerator{std::random_device{}()};

	quint32 getRandomUInt32() {
		std::uniform_int_distribution<quint32> d;
		return d(randomGenerator);
	}
}
