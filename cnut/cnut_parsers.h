#ifndef NUT_CMD_PARSERS_H
#define NUT_CMD_PARSERS_H

#include "cnut_commands.h"

namespace cnut {
	int dispatchCommands(CommandList commands, QDBusConnection connection);
	CommandList toCommandList(QStringList &commands);
}

#endif
