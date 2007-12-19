#ifndef NUT_CMD_PARSERS_H
#define NUT_CMD_PARSERS_H
#include "cnut_commands.h"
namespace cnut {

	void help();
	int dispatchCommands(CommandList commands, QDBusConnection * connection);
	NUT_COMMANDS cmdStrToNUT_CMDS(QString cmd);
	CommandList toCommandList(QStringList &commands);
}
#endif
