//TODO: define return values
#include "nut_cmd_parsers.h"

namespace nut_cmd {
	
	void help() {
		print(QString("--listDevices %1").arg(QObject::tr("Lists available devices")));
		print(QString("--device <devicename> %1").arg(QObject::tr("Choose device")));
		print(QString("--environment <environment name> %1").arg(QObject::tr("Choose environment in given device")));
		print(QString("--listEnvironments %1").arg(QObject::tr("Lists environments of device set by --Device")));
		print(QString("--enable %1").arg(QObject::tr("Enables given device")));
		print(QString("--disable %1").arg(QObject::tr("Disables given device")));
		print(QString("--type %1").arg(QObject::tr("returns type of given device")));
		print(QString("--state %1").arg(QObject::tr("returns state of given device or environment")));
		print(QString("--activeEnvironment %1").arg(QObject::tr("returns active Environment of given device")));
		print(QString("--setEnvironment <environment name> %1").arg(QObject::tr("Activates the environment on the given device")));
		print(QString("--setEnvironment <index> %1").arg(
		QObject::tr("Activates the environment on the given device. Index = line number from --listEnvironments starting with 0"))
		);
		print(""); print("");
		print(QObject::tr("Examples:"));
		print(QString("nut_cmd --device eth0 --enable %1").arg(QObject::tr("Enables device eth0")));
		print(QString("nut_cmd --device eth0 --state %1").arg(QObject::tr("Shows state of device eth0")));
		print(QString("nut_cmd --device eth0 --environment \"home\" --state %1").arg(QObject::tr("Shows state of environment home of device eth0")));
	}
	
	int dispatchCommands(CommandList commands, QDBusConnection * connection) {
		//Commands are sorted in this order:
		//--listDevices
		//--device <> --listEnvironments
		//--device <> --state
		//--device <> --environment <>
		//--device <> --environment <> --state
		
		QString deviceName;
		QString environmentName;
		QString devPath;
		QString envPath;
		//Now dispatch the commands:
		foreach(NutCommand cmd, commands) {
// 			qDebug() << "Dispatching a command:";
// 			qDebug() << QString("Commands value: %1").arg(cmd.value); 
			if (CMD_HELP == cmd.command) {
				help();
				return RETVAL_SUCCESS;
			}
			//no specs.
			else if (CMD_LIST_DEVICES == cmd.command) {
				print(listDeviceNames(connection));
				return RETVAL_SUCCESS;
			}
			//specs: device
			else if (CMD_DEVICE == cmd.command) {
				deviceName = cmd.value;
				devPath = getDevicePathByName(connection,deviceName);
				if (devPath.isEmpty()) {
					print(QObject::tr("Device not found"));
					return RETVAL_DEVICE_NOT_FOUND;
				}
			}
			else if (CMD_LIST_ENVIRONMENTS == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(listEnvironmentNames(connection,devPath));
				}
				return RETVAL_SUCCESS;
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
			else if (CMD_TYPE == cmd.command) {
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getDeviceType(connection,devPath));
				}
				return RETVAL_SUCCESS;
			}
			else if (CMD_STATE == cmd.command) { //no env set.
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getDeviceState(connection,devPath));
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

						qDebug() << QString("Parsed setEnvironment with: %1").arg(name);
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
					envPath == getEnvironmentPathByName(connection,devPath,cmd.value);
				}
			}
			else if (CMD_STATE == cmd.command) { //env set.
				if (devPath.isEmpty()) {
					print(QObject::tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else if (envPath.isEmpty()) {
					print(QObject::tr("No environment specified"));
					return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
				}
				else {
					getEnvironmentState(connection,envPath);
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
		if ("--listDevices" == cmd) {
			return CMD_LIST_DEVICES;
		}
		else if ("--device" == cmd) {
			return CMD_DEVICE;
		}
		else if ("--listEnvironments" == cmd) {
			return CMD_LIST_ENVIRONMENTS;
		}
		else if ("--enable" == cmd) {
			return CMD_ENABLE;
		}
		else if ("--disable" == cmd) {
			return CMD_DISABLE;
		}
		else if ("--type" == cmd) {
			return CMD_TYPE;
		}
		else if ("--state" == cmd) {
			return CMD_STATE;
		}
		else if ("--activeEnvironment" == cmd) {
			return CMD_ACTIVE_ENVIRONMENT;
		}
		else if ("--setEnvironment" == cmd) {
			return CMD_SET_ENVIRONMENT;
		}
		else if ("--environment" == cmd) {
			return CMD_ENVIRONMENT;
		}
		else if ("--help" == cmd) {
			return CMD_HELP;
		}
		else {
			return CMD_UNKNOWN;
		}
	}

	inline bool cmdNeedsArguments(NUT_COMMANDS cmd) {
		return (CMD_DEVICE == cmd || CMD_ENVIRONMENT == cmd || CMD_SET_ENVIRONMENT == cmd);
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
			if (0 == cmdIter->indexOf("--")) { //Command
				//we have a command
				command.command = cmdStrToNUT_CMDS(*cmdIter);
				//Check if it needs arguments:
				if (cmdNeedsArguments(command.command)) {
					++cmdIter;
					if (cmdIter != commands.end()) { //check if last arg
						if (0 == cmdIter->indexOf("--")) { //again a command => value not specified
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
					cmdList.append(command);
					cmdIter++;
				}
			}
			else {
				++cmdIter;
			}
			
		}
		qSort(cmdList);
		return cmdList;
	}

}
