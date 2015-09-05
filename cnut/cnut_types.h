#ifndef NUT_CMD_TYPES_H
#define NUT_CMD_TYPES_H

#include <libnutcommon/common.h>
#include <QList>

namespace cnut {
	enum NUT_COMMANDS : quint32 {
		CMD_UNKNOWN = 0,
		CMD_HELP,
		CMD_DEVICE,
		CMD_ENABLE,
		CMD_DISABLE,
		CMD_ACTIVE_ENVIRONMENT,
		CMD_SET_ENVIRONMENT,
		CMD_ENVIRONMENT,
		CMD_SELECTABLE,
		CMD_WITH_INDEX,
		CMD_INTERFACE,
		CMD_PROPERTIES,
		CMD_TYPE,
		CMD_STATE,
		CMD_LIST,
	};

	struct NutCommand {
		NUT_COMMANDS command;
		QString value;
		friend inline bool operator<(NutCommand a, NutCommand b) { return a.command < b.command; }
	};

	typedef QList<NutCommand> CommandList;

	enum NUT_CMD_RETVALS : quint32 {
		RETVAL_SUCCESS = 0,
		RETVAL_UNKNOWN_COMMAND,
		RETVAL_DBUS_ERROR,
		RETVAL_NUTS_OFF,
		RETVAL_DEVICE_NOT_FOUND,
		RETVAL_DEVICE_NOT_SPECIFIED,
		RETVAL_ENVIRONMENT_NOT_FOUND,
		RETVAL_ENVIRONMENT_NOT_SPECIFIED,
		RETVAL_INTERFACE_NOT_SPECIFIED,
		RETVAL_INTERFACE_NOT_FOUND,
	};
}

#endif
