
#ifndef _NUTS_CONFIG_H
#define _NUTS_CONFIG_H

#include <libnutcommon/config.h>

namespace nuts {
	libnutcommon::Config parseConfig(QString const& filename);
}

#endif
