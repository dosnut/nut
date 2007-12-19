//TODO: define return values
#include "cnut_parsers.h"

namespace cnut {
	
	void help() {
		print(QString("--list (-l) %1").arg(QObject::tr("Lists available devices")));
		print(QString("--device (-D) <devicename> %1").arg(QObject::tr("Choose device")));
		print(QString("--environment (-E) <environment name> %1").arg(QObject::tr("Choose environment in given device")));
		print(QString("--environment (-E) <environment index> %1").arg(QObject::tr("Choose environment in given device")));
		print(QString("--interface <interface index> (-I) %1").arg(QObject::tr("Choose interface")));
		print(QString("--device <devname> --list %1").arg(QObject::tr("Lists environments of device set by --Device")));
		print(QString("--enable (-1) %1").arg(QObject::tr("Enables given device")));
		print(QString("--disable (-0) %1").arg(QObject::tr("Disables given device")));
		print(QString("--type (-t) %1").arg(QObject::tr("returns type of given device or interface")));
		print(QString("--state (-s) %1").arg(QObject::tr("returns state of given device or environment or interface")));
		print(QString("--activeEnvironment (-a) %1").arg(QObject::tr("returns active Environment of given device")));
		print(QString("--setEnvironment (-S) <environment name> %1").arg(QObject::tr("Activates the environment on the given device")));
		print(QString("--properties (-p) %1").arg(QObject::tr("Returns interface properties (one ip per line): ip,netmask,gateway,dns-servers")));
		print(QString("--setEnvironment (-S) <index> %1").arg(
		QObject::tr("Activates the environment on the given device. Index = line number from --listEnvironments starting with 0")));
		print(QString("--selectable %1").arg("returns if an environment is selected,selectable or not selectable"));
		print(""); print("");
		print(QObject::tr("Examples:"));
		print(QString("cnut --device eth0 --enable %1").arg(QObject::tr("Enables device eth0")));
		print(QString("cnut --device eth0 --state %1").arg(QObject::tr("Shows state of device eth0")));
		print(QString("cnut --device eth0 --environment \"home\" --state %1").arg(QObject::tr("Shows state of environment home of device eth0")));
	}
	
	int dispatchCommands(CommandList commands, QDBusConnection * connection) {
		//Commands are sorted in this order:
		//--listDevices
		//--device <> --listEnvironments
		//--device <> --state
		//--device <> --environment <>
		//--device <> --environment <> --state
		
		QString devPath;
		QString envPath;
		QString ifPath;
		bool doEnv = false;
		bool doIf = false;
		bool doDev = false;
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
				if (devPath.isEmpty()) {
					print(QObject::tr("Device not found"));
					return RETVAL_DEVICE_NOT_FOUND;
				}
			}
			else if (CMD_ENABLE == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					enableDevice(connection,devPath);
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_DISABLE == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					disableDevice(connection,devPath);
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_ACTIVE_ENVIRONMENT == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getActiveEnvironment(connection,devPath));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_SET_ENVIRONMENT == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
// 					qDebug() << QString("Parsed setEnvironment with: %1").arg(cmd.value);
					if (cmd.value.contains("\"")) { //Check if index or name
						QString name = cmd.value;
						name.chop(1);
						name.remove(0,1);

// 						qDebug() << QString("Parsed setEnvironment with: %1").arg(name);
						envPath = getEnvironmentPathByName(connection,devPath,name);
						if (envPath.isEmpty()) {
							print(QObject::tr("Environment not found"));
							return RETVAL_ENVIRONMENT_NOT_FOUND;
						}
						else {
							setEnvironment(connection,devPath,envPath);
						}
					}
					else { //index
						bool ok;
						qint32 index = cmd.value.toInt(&ok);
						if (ok) {
							if (!setEnvironment(connection,devPath,index)) {
								return RETVAL_SUCCESS;
							}
						}
						else { //QT strips ", so let's try it with name:
							envPath = getEnvironmentPathByName(connection,devPath,cmd.value);
							if (envPath.isEmpty()) {
								print(QObject::tr("Environment not found"));
								return RETVAL_ENVIRONMENT_NOT_FOUND;
							}
							else {
								setEnvironment(connection,devPath,envPath);
								return RETVAL_SUCCESS;
							}
						}
					}
				}
			}
			else if (CMD_ENVIRONMENT == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					doEnv = true;
					bool ok;
					qint32 index = cmd.value.toInt(&ok);
					if (ok) { //try number first
						envPath = getEnvironmentPathByIndex(connection,devPath,index);
					}
					else {
						envPath = getEnvironmentPathByName(connection,devPath,cmd.value);
					}
				}
			}
			else if (CMD_SELECTABLE == cmd.command) {
				if (envPath.isEmpty()) {
					print(QObject::tr("No environment specified"));
					return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
				}
				else {
					print(getEnvironmentSelectable(connection,envPath));
					return RETVAL_SUCCESS;
				}
			}
			else if (CMD_INTERFACE == cmd.command) {
				if (envPath.isEmpty()) {
					print(QObject::tr("No environment specified"));
					return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
				}
				else {
					doIf = true;
					bool ok;
					qint32 index = cmd.value.toInt(&ok);
					if (ok) { //try number first
						ifPath = getInterfacePathByIndex(connection,envPath,index);
					}
					else {
						print(QObject::tr("Interface not found"));
						return RETVAL_INTERFACE_NOT_FOUND;
					}
				}
			}
			else if (CMD_PROPERTIES == cmd.command) {
				if (ifPath.isEmpty()) {
					print(QObject::tr("No interface specified"));
					return RETVAL_INTERFACE_NOT_SPECIFIED;
				}
				else {
					print(getInterfaceProperties(connection,ifPath));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_STATE == cmd.command) { //check which state is meant
				if (doIf) { //interface state
					if (ifPath.isEmpty()) {
						print(QObject::tr("No interface specified"));
						return RETVAL_INTERFACE_NOT_SPECIFIED;
					}
					else {
						print(getInterfaceState(connection,ifPath));
					}
				}
				else if (doEnv) { //env state
					if (devPath.isEmpty()) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else if (envPath.isEmpty()) {
						print(QObject::tr("No environment specified"));
						return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
					}
					else {
						print(getEnvironmentState(connection,envPath));
					}
				}
				else { //dev state
					if (devPath.isEmpty()) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else {
						print(getDeviceState(connection,devPath));
					}
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_TYPE == cmd.command) { //check which type is meant
				if (doIf) {
					if (ifPath.isEmpty()) {
						print(QObject::tr("No device specified"));
						return RETVAL_INTERFACE_NOT_SPECIFIED;
					}
					else {
						print(getInterfaceType(connection,ifPath));
					}
				}
				else {
					if (devPath.isEmpty()) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else {
						print(getDeviceType(connection,devPath));
					}
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_LIST == cmd.command) {
				//Check which list is meant
				if (doEnv) { //List interfaces
					if (envPath.isEmpty()) {
						print(QObject::tr("No environment specified"));
						return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
					}
					else {
						print(listInterfaceIndexes(connection,envPath));
					}
				}
				else if (doDev) { //list environments
					if (devPath.isEmpty()) {
						print(QObject::tr("No device specified"));
						return RETVAL_DEVICE_NOT_SPECIFIED;
					}
					else {
						print(listEnvironmentNames(connection,devPath));
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
	
	NUT_COMMANDS cmdStrToNUT_CMDS(QString cmd) {
// 		qDebug() << QString("Current parsed command: %1").arg(cmd);
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

	inline bool cmdNeedsArguments(NUT_COMMANDS cmd) {
		return (CMD_DEVICE == cmd || CMD_ENVIRONMENT == cmd || CMD_SET_ENVIRONMENT == cmd || CMD_INTERFACE == cmd);
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
