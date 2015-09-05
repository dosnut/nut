//TODO: define return values
#include "cnut_parsers.h"

#include <QTextStream>

namespace cnut {
	namespace {
		bool cmdNeedsArguments(NUT_COMMANDS cmd) {
			switch (cmd) {
			case CMD_DEVICE:
			case CMD_ENVIRONMENT:
			case CMD_SET_ENVIRONMENT:
			case CMD_INTERFACE:
				return true;
			default:
				return false;
			}
		}

		NUT_COMMANDS cmdStrToNUT_CMDS(QString cmd) {
//			qDebug() << QString("Current parsed command: %1").arg(cmd);
			if ("--list" == cmd || "-l" == cmd) {
				return CMD_LIST;
			}
			else if ("--device" == cmd || "-D" == cmd) {
				return CMD_DEVICE;
			}
			else if ("--enable" == cmd || "-1" == cmd) {
				return CMD_ENABLE;
			}
			else if ("--disable" == cmd || "-0" == cmd) {
				return CMD_DISABLE;
			}
			else if ("--type" == cmd || "-t" == cmd) {
				return CMD_TYPE;
			}
			else if ("--state" == cmd || "-s" == cmd) {
				return CMD_STATE;
			}
			else if ("--activeEnvironment" == cmd || "-a" == cmd) {
				return CMD_ACTIVE_ENVIRONMENT;
			}
			else if ("--setEnvironment" == cmd || "-S" == cmd) {
				return CMD_SET_ENVIRONMENT;
			}
			else if ("--environment" == cmd || "-E" == cmd) {
				return CMD_ENVIRONMENT;
			}
			else if("--selectable" == cmd) {
				return CMD_SELECTABLE;
			}
			else if("--with-index" == cmd) {
				return CMD_WITH_INDEX;
			}
			else if("--interface" == cmd || "-I" == cmd) {
				return CMD_INTERFACE;
			}
			else if ("--properties" == cmd || "-p" == cmd) {
				return CMD_PROPERTIES;
			}
			else if ("--help" == cmd || "-h" == cmd) {
				return CMD_HELP;
			}
			else {
				return CMD_UNKNOWN;
			}
		}

		void print_out(QTextStream& out, QString const& str) {
			out << str << "\n";
		}
		void print_out(QTextStream& out, QStringList const& list) {
			for (auto const& s: list) {
				print_out(out, s);
			}
		}
		template<typename... Args>
		inline void print(Args&&... args) {
			QTextStream out { stdout };
			print_out(out, std::forward<Args>(args)...);
		}

		void help() {
			QTextStream(stdout)
				<< QString("--list (-l) %1").arg(QObject::tr("Lists available devices/environments/interfaces")) << "\n"
				<< QString("--device (-D) <devicename> %1").arg(QObject::tr("Choose device")) << "\n"
				<< QString("--environment (-E) <environment name> %1").arg(QObject::tr("Choose environment in given device")) << "\n"
				<< QString("--environment (-E) <environment index> %1").arg(QObject::tr("Choose environment in given device")) << "\n"
				<< QString("--interface <interface index> (-I) %1").arg(QObject::tr("Choose interface")) << "\n"
				<< QString("--device <devname> --list %1").arg(QObject::tr("Lists environments of device set by --Device")) << "\n"
				<< QString("--enable (-1) %1").arg(QObject::tr("Enables given device")) << "\n"
				<< QString("--disable (-0) %1").arg(QObject::tr("Disables given device")) << "\n"
				<< QString("--type (-t) %1").arg(QObject::tr("returns type of given device")) << "\n"
				<< QString("--state (-s) %1").arg(QObject::tr("returns state of given device or environment or interface")) << "\n"
				<< QString("--activeEnvironment (-a) %1").arg(QObject::tr("returns active Environment of given device")) << "\n"
				<< QString("--setEnvironment (-S) <environment name> %1").arg(QObject::tr("Activates the environment on the given device")) << "\n"
				<< QString("--properties (-p) %1").arg(QObject::tr("Returns interface properties (one ip per line): ip,netmask,gateway,dns-servers")) << "\n"
				<< QString("--setEnvironment (-S) <index> %1").arg(QObject::tr("Activates the environment on the given device.")) << "\n"
				<< QString("--selectable %1").arg("returns if an environment is selected,selectable or not selectable") << "\n"
				<< QString("--with-index %1").arg(QObject::tr("In conjunction with --list and --device this will list environments with index in front of name")) << "\n"
				<< "\n"
				<< "\n"
				<< QObject::tr("Examples:") << "\n"
				<< QString("cnut --device eth0 --enable %1").arg(QObject::tr("Enables device eth0")) << "\n"
				<< QString("cnut --device eth0 --state %1").arg(QObject::tr("Shows state of device eth0")) << "\n"
				<< QString("cnut --device eth0 --environment \"home\" --state %1").arg(QObject::tr("Shows state of environment home of device eth0")) << "\n"
				;
		}
	}

	int dispatchCommands(CommandList commands, QDBusConnection connection) {
		//Commands are sorted in this order:
		//--listDevices
		//--device <> --listEnvironments
		//--device <> --state
		//--device <> --environment <>
		//--device <> --environment <> --state

		OptionalQDBusObjectPath devPath;
		OptionalQDBusObjectPath envPath;
		OptionalQDBusObjectPath ifPath;

		bool doEnv = false;
		bool doIf = false;
		bool doDev = false;
		bool envNamesWithIndex = false;
		//Now dispatch the commands:
		foreach(NutCommand cmd, commands) {
// 			qDebug() << "Dispatching a command:";
// 			qDebug() << QString("Commands value: %1").arg(cmd.value);
			if (CMD_UNKNOWN == cmd.command) {
				print(QString("Command not found: %1").arg(cmd.value));
				help();
				return RETVAL_UNKNOWN_COMMAND;
			}
			else if (CMD_HELP == cmd.command) {
				help();
				return RETVAL_SUCCESS;
			}
			//specs: device
			else if (CMD_DEVICE == cmd.command) {
				devPath = getDevicePathByName(connection,cmd.value);
				doDev = true;
				if (!devPath) {
					print(QObject::tr("Device not found"));
					return RETVAL_DEVICE_NOT_FOUND;
				}
			}
			else if (CMD_ENABLE == cmd.command) {
				if (!devPath) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					enableDevice(connection,devPath.objectPath());
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_DISABLE == cmd.command) {
				if (!devPath) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					disableDevice(connection,devPath.objectPath());
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_ACTIVE_ENVIRONMENT == cmd.command) {
				if (!devPath) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getActiveEnvironment(connection,devPath.objectPath()));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_SET_ENVIRONMENT == cmd.command) {
				if (!devPath) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
// 					qDebug() << QString("Parsed setEnvironment with: %1").arg(cmd.value);
					if (cmd.value.isEmpty()) {
						return RETVAL_ENVIRONMENT_NOT_FOUND;
					}
					else if (cmd.value.contains("\"")) { //Check if index or name
						QString name = cmd.value;
						name.chop(1);
						name.remove(0,1);

// 						qDebug() << QString("Parsed setEnvironment with: %1").arg(name);
						envPath = getEnvironmentPathByName(connection,devPath.objectPath(),name);
						if (!envPath) {
							print(QObject::tr("Environment not found"));
							return RETVAL_ENVIRONMENT_NOT_FOUND;
						}
						else {
							setEnvironment(connection,devPath.objectPath(),envPath.objectPath());
						}
					}
					else { //index
						bool ok;
						qint32 index = cmd.value.toInt(&ok);
						if (ok) {
							setEnvironment(connection,devPath.objectPath(),index);
							return RETVAL_SUCCESS;
						}
						else { //QT strips ", so let's try it with name:
							envPath = getEnvironmentPathByName(connection,devPath.objectPath(),cmd.value);
							if (!envPath) {
								print(QObject::tr("Environment not found"));
								return RETVAL_ENVIRONMENT_NOT_FOUND;
							}
							else {
								setEnvironment(connection,devPath.objectPath(),envPath.objectPath());
								return RETVAL_SUCCESS;
							}
						}
					}
				}
			}
			else if (CMD_ENVIRONMENT == cmd.command) {
				if (!devPath) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					doEnv = true;
					bool ok;
					qint32 index = cmd.value.toInt(&ok);
					if (ok) { //try number first
						envPath = getEnvironmentPathByIndex(connection,devPath.objectPath(),index);
					}
					else {
						envPath = getEnvironmentPathByName(connection,devPath.objectPath(),cmd.value);
					}
				}
			}
			else if (CMD_SELECTABLE == cmd.command) {
				if (!envPath) {
					print(QObject::tr("No environment specified"));
					return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
				}
				else {
					print(getEnvironmentSelectable(connection,envPath.objectPath()));
					return RETVAL_SUCCESS;
				}
			}
			else if (CMD_WITH_INDEX == cmd.command) {
				envNamesWithIndex = true;
			}
			else if (CMD_INTERFACE == cmd.command) {
				if (!envPath) {
					print(QObject::tr("No environment specified"));
					return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
				}
				else {
					doIf = true;
					bool ok;
					qint32 index = cmd.value.toInt(&ok);
					if (ok) { //try number first
						ifPath = getInterfacePathByIndex(connection,envPath.objectPath(),index);
					}
					else {
						print(QObject::tr("Interface not found"));
						return RETVAL_INTERFACE_NOT_FOUND;
					}
				}
			}
			else if (CMD_PROPERTIES == cmd.command) {
				if (!ifPath) {
					print(QObject::tr("No interface specified"));
					return RETVAL_INTERFACE_NOT_SPECIFIED;
				}
				else {
					print(getInterfaceProperties(connection,ifPath.objectPath()));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_STATE == cmd.command) { //check which state is meant
				if (doIf) { //interface state
					if (!ifPath) {
						print(QObject::tr("No interface specified"));
						return RETVAL_INTERFACE_NOT_SPECIFIED;
					}
					else {
						print(getInterfaceState(connection,ifPath.objectPath()));
					}
				}
				else if (doEnv) { //env state
					if (!devPath) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else if (!envPath) {
						print(QObject::tr("No environment specified"));
						return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
					}
					else {
						print(getEnvironmentActive(connection,envPath.objectPath()));
					}
				}
				else { //dev state
					if (!devPath) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else {
						print(getDeviceState(connection,devPath.objectPath()));
					}
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_TYPE == cmd.command) {
				if (!devPath) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getDeviceType(connection,devPath.objectPath()));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_LIST == cmd.command) {
				//Check which list is meant
				if (doEnv) { //List interfaces
					if (!envPath) {
						print(QObject::tr("No environment specified"));
						return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
					}
					else {
						print(listInterfaceIndexes(connection,envPath.objectPath()));
					}
				}
				else if (doDev) { //list environments
					if (!devPath) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else {
						print(listEnvironmentNames(connection, devPath.objectPath(), envNamesWithIndex));
					}
				}
				else { //List devices
					print(listDeviceNames(connection));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_UNKNOWN == cmd.command) {
				help();
				return RETVAL_SUCCESS;
			}
		}
		return RETVAL_SUCCESS;
	}

	//Transform argument list as passed by QApplication::arguments to own cmd list
	CommandList toCommandList(QStringList &commands) {
		//First transform into command list, then sort command list
// 		qDebug() << QString("argument list size is %1").arg(QString::number(commands.size()));
		CommandList cmdList;
		if (1 == commands.size()) {
			NutCommand command;
			command.command = CMD_HELP;
			cmdList.append(command);
			return cmdList;
		}
		QStringList::iterator cmdIter = commands.begin();
		++cmdIter; //First is prog name
		while (cmdIter != commands.end()) {
			NutCommand command;
			if (0 == cmdIter->indexOf("--") || 0 == cmdIter->indexOf("-")) { //Command
				//we have a command
				command.command = cmdStrToNUT_CMDS(*cmdIter);
				//Check if it needs arguments:
				if (cmdNeedsArguments(command.command)) {
					++cmdIter;
					if (cmdIter != commands.end()) { //check if last arg
						if (0 == cmdIter->indexOf("--") || 0 == cmdIter->indexOf("-")) { //again a command => value not specified
							command.value = QString(); // Insert empty string for error reporting
						}
						else {
							command.value = *cmdIter;
							cmdList.append(command);
							++cmdIter; //parse next one
						}
					}
					else { //end of arg list
						command.value = QString(); //Insert for error reporting
						cmdList.append(command);
					}
				}
				else { //insert command and do the next one
					if (CMD_UNKNOWN == command.command) { //If command is unknow
						command.value = *cmdIter;
					}
					cmdList.append(command);
					cmdIter++;
				}
			}
			else { //we have an unknown command:
				command.command = CMD_UNKNOWN;
				command.value = *cmdIter;
				cmdList.append(command);
				++cmdIter;
			}

		}
		qStableSort(cmdList);
		return cmdList;
	}

}
