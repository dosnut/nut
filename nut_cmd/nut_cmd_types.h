#include NUT_CMD_TYPES_H
#ifndef NUT_CMD_TYPES_H
#include <QList>
inline operator<(nut_cmd::NutCommand one, nut_cmd::NutCommand two) { return one.command < two.command;}

namespace nut_cmd {
	//In order to sort, we define an order on our commands.
	//But we don't really want to define comparison operators.
	//Let's just define an bijective function between our commands and the set {0..n} and use int's operators
	typedef enum {CMD_LIST_DEVICES,CMD_DEVICE,CMD_LIST_ENVIRONMENTS,CMD_ENABLE,CMD_DISABLE,CMD_TYPE,CMD_ACTIVE_ENVIRONMENT,CMD_SET_ENVIRONMENT,CMD_ENVIRONMENT,CMD_STATE,CND_UNKNOWN} NUT_COMMANDS;
	struct NutCommand {
		NUT_COMMANDS command;
		QString value;
	};
	typedef CommandList QList<NutCommand>;
	
	typedef enum {
	RETVAL_SUCCESS=0,
	RETVAL_DBUS_ERROR,RETVAL_NUTS_OFF
	RETVAL_DEVICE_NOT_FOUND, RETVAL_DEVICE_NOT_SPECIFIED, RETVAL_ENVIRONMENT_NOT_FOUND,RETVAL_ENVIRONMENT_NOT_SPECIFIED
	} NUT_CMD_RETVALS;
}
#endif