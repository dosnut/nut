#ifndef _NUTS_CONFIG_H
#define _NUTS_CONFIG_H

#pragma once

#include <libnutcommon/config.h>

namespace nuts {
	libnutcommon::Config parseConfig(QString const& filename);
}

#endif /* _NUTS_CONFIG_H */
