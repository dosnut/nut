#ifndef _NUTS_RANDOM_H
#define _NUTS_RANDOM_H

#pragma once

#include <QtGlobal>
#include <random>

namespace nuts {
	extern std::mt19937 randomGenerator;

	quint32 getRandomUInt32();
}

#endif /* _NUTS_RANDOM_H */
