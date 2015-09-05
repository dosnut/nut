#ifndef LIBNUTCLIENT_CLIBNUT_H
#define LIBNUTCLIENT_CLIBNUT_H

#include <QObject>

#include "libnutcommon/common.h"

namespace libnutclient {
	QString toStringTr(libnutcommon::DeviceState state);
	QString toStringTr(libnutcommon::DeviceType type);
	QString toStringTr(libnutcommon::InterfaceState state);

	QString toString(QDBusError const& error);
}

#endif
