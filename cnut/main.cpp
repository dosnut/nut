#include "main.h"
using namespace cnut;
int main(int argc, char * argv[]) {

	libnutcommon::init();
	QCoreApplication app(argc, argv); //just for parsing, maybe this should be changed (overhead?)


	//Open connection to DBus:
	QDBusConnection connection(QDBusConnection::systemBus());
	//Check if nuts is running:
	QDBusReply<bool> reply = connection.interface()->isServiceRegistered(NUT_DBUS_URL);
	if (reply.isValid()) {
		if (!reply.value()) {
			return RETVAL_NUTS_OFF;
		}
	}
	else {
		return RETVAL_DBUS_ERROR;
	}

	//Nuts is running, now let's parse our commands

	QStringList rawCmdList = app.arguments();
	
	//Transform commands into command list:
	CommandList cmdList = toCommandList(rawCmdList);
	return dispatchCommands(cmdList,&connection);
}
