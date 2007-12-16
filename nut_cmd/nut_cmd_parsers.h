#ifndef NUT_CMD_PARSERS_H
#define NUT_CMD_PARSERS_H
#include "nut_cmd_commands.h"
namespace nut_cmd {
	void print(QStringList list);
	void print(QString str);
	void help();
	int dispatchCommands(CommandList commands, QDBusConnection * connection);
	NUT_COMMANDS cmdStrToNUT_CMDS(QString cmd);
	CommandList toCommandList(QList<RawCommand> commands);
}
#endif