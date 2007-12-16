//TODO: define return values
#include "nut_cmd_parsers.h"

namespace nut_cmd {
	
	void print(QStringList list) {
		foreach(QString i, list) {
			cout << i << endl;
		}
	}
	void print(QString str) {
		cout << str << endl;
	}

	void help() {
		print(QString("--listDevices %1").arg(tr("Lists available devices"));
		print(QString("--device <devicename>".arg(tr("Choose device"));
		print(QString("environment <environment name>".arg(tr("Choose environment in given device"));
		print(QString("listEnvironments".arg(tr("Lists environments of device set by --Device"));
		print(QString("enable".arg(tr("Enables given device"));
		print(QString("disable".arg(tr("Disables given device"));
		print(QString("type".arg(tr("returns type of given device"));
		print(QString("state".arg(tr("returns state of given device or environment"));
		print(QString("activeEnvironment".arg(tr("returns active Environment of given device"));
		print(QString("setEnvironment <environment name>".arg(tr("Activates the environment on the given device"));
		print(QString("setEnvironment <index>".arg(tr("Activates the environment on the given device"));
		print(); print();
		print(tr("Examples:"));
		print(QString("nut_cmd --device eth0 --enable %1").arg(tr("Enables device eth0"));
		print(QString("nut_cmd --device eth0 --state %1").arg(tr("Shows state of device eth0"));
		print(QString("nut_cmd --device eth0 --environment \"home\" --state").arg(tr("Shows state of environment home of device eth0"));
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
		foreach(RawCommand cmd, commands) {
			//no specs.
			if (CMD_LIST_DEVICES == cmd.command) {
				print(listDeviceNames(connection));
				return RETVAL_SUCCESS;
			}
			//specs: device
			else if (CMD_DEVICE == cmd.command) {
				deviceName = cmd.value;
				devPath = getDevicePathByName(connection,name);
				if (devPath.isEmpty()) {
					print(tr("Device not found"));
					return RETVAL_DEVICE_NOT_FOUND;
				}
			}
			else if (CMD_LIST_ENVIRONMENTS == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(listEnvironmentNames(connection,devPath));
				}
				return;
			}
			else if (CMD_ENABLE == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(enableDevice(connection,devPath));
				}
				return;
			}
			else if (CMD_DISABLE == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(disableDevice(connection,devPath));
				}
				return;
			}
			else if (CMD_TYPE == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getDeviceType(connection,devPath));
				}
				return;
			}
			else if (CMD_STATE == cmd.command) { //no env set.
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getDeviceState(connection,devPath));
				}
				return;
			}
			else if (CMD_ACTIVE_ENVIRONMENT == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					print(getActiveEnvironment(connection,devPath));
				}
				return;
			}
			else if (CMD_SET_ENVIRONMENT == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					if (cmd.value.contains("\"")) { //Check if index or name
						envPath = getEnvironmentPathByName(connection,devPath,cmd.value);
						if (envPath.isEmpty()) {
							print(tr("Environment not found"));
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
								return;
							}
						}
						else {
							return;
						}
					}
				}
			}
			else if (CMD_ENVIRONMENT == cmd.command) {
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else {
					envPath == getEnvironmentPathByName(connection,devPath,cmd.command);
				}
			}
			else if (CMD_STATE == cmd.command) { //env set.
				if (devPath.isEmpty()) {
					print(tr("No device specified"));
					return RETVAL_DEVICE_NOT_SPECIFIED;
				}
				else if (envPath.isEmpty()) {
					print(tr("No environment specified"));
					return RETVAL_ENVIRONMENT_NOT_SPECIFIED;
				}
				else {
					print(getEnvironmentState(connection,devPath,envPath));
				}
				return;
			}
			else if (CMD_UNKNOWN = cmd.command) {
				help();
				return;
			}
		}
	}
	
	NUT_COMMANDS cmdStrToNUT_CMDS(QString cmd) {
		if ("listDevices" == cmd) {
			return CMD_LIST_DEVICES;
		}
		else if ("device" == cmd) {
			return CMD_DEVICE;
		}
		else if ("listEnvironments" == cmd) {
			return CMD_LIST_ENVIRONMENTS;
		}
		else if ("enable" == cmd) {
			return CMD_ENABLE;
		}
		else if ("disable" == cmd) {
			return CMD_DISABLE;
		}
		else if ("type" == cmd) {
			return CMD_TYPE;
		}
		else if ("state" == cmd) {
			return CMD_STATE;
		}
		else if ("activeEnvironment" == cmd) {
			return CMD_ACTIVE_ENVIRONMENT;
		}
		else if ("setEnvironment" == cmd) {
			return CMD_SET_ENVIRONMENT;
		}
		else if ("environment" == cmd) {
			return CMD_ENVIRONMENT;
		}
		else {
			return CMD_UNKNOWN;
		}
	}

	inline bool cmdNeedsArguments(NUT_COMMANDS cmd) {
		return (CMD_DEVICE == cmd || CMD_ENVIRONMENTS == cmd || CMD_SET_ENVIRONMENT == cmd);
	}

	//Transform argument list as passed by QApplication::arguments to own cmd list
	CommandList toCommandList(QStringList &commands) {
		//First transform into command list, then sort command list
		CommandList cmdList;
		QStringList::iterator cmdIter = commands.begin();
		++cmdIter; //First is prog name
		while (cmdIter != commands.end()) {
			NutCommand command;
			if (0 == i->indexOf("--")) { //Command
				//we have a command
				command.command = cmdStrToNUT_CMDS(*cmdIter);
				//Check if it needs arguments:
				if (cmdNeedsArguments(command.command)) {
					++cmdIter;
					if (cmdIter != commands.end()) { //check if last arg
						if (0 == *cmdIter.indexOf("--")) { //again a command => value not specified
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