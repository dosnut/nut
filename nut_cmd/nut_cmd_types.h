#ifndef NUT_CMD_TYPES_H
#define NUT_CMD_TYPES_H
#include <QList>
namespace nut_cmd {
	//In order to sort, we define an order on our commands.
	//But we don't really want to define comparison operators.
	//Let's just define an bijective function between our commands and the set {0..n} and use int's operators
	typedef enum {CMD_HELP,CMD_DEVICE,CMD_ENABLE,CMD_DISABLE,CMD_ACTIVE_ENVIRONMENT,CMD_SET_ENVIRONMENT,CMD_ENVIRONMENT,CMD_INTERFACE,CMD_PROPERTIES,CMD_TYPE,CMD_STATE,CMD_LIST,CMD_UNKNOWN} NUT_COMMANDS;
	struct NutCommand {
		NUT_COMMANDS command;
		QString value;
	};
	typedef QList<NutCommand> CommandList;
	
	typedef enum {
	RETVAL_SUCCESS=0,
	RETVAL_DBUS_ERROR, RETVAL_NUTS_OFF,
	RETVAL_DEVICE_NOT_FOUND, RETVAL_DEVICE_NOT_SPECIFIED, RETVAL_ENVIRONMENT_NOT_FOUND,RETVAL_ENVIRONMENT_NOT_SPECIFIED,RETVAL_INTERFACE_NOT_SPECIFIED,RETVAL_INTERFACE_NOT_FOUND
	} NUT_CMD_RETVALS;
}
inline bool operator<(nut_cmd::NutCommand one, nut_cmd::NutCommand two) { return one.command < two.command;}
#endif
