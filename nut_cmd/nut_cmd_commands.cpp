#include "nut_cmd_commands.h"
namespace nut_cmd {

	void print(QStringList list) {
		foreach(QString i, list) {
			std::cout << i.toLatin1().data();
			std::cout.put('\n');
		}
	}
	void print(QString str) {
		std::cout << str.toLatin1().data();
		std::cout.put('\n');
	}

	//"Public functions"
	QStringList listDeviceNames(QDBusConnection * connection) {
		//Get *connection to device manager
		DBusDeviceManagerInterface devmgr(NUT_DBUS_URL, "/manager",*connection, 0);
		QDBusReply<QList<QDBusObjectPath> > replydevs = devmgr.getDeviceList();
		QStringList devList;
		QString devPath;
		if (replydevs.isValid()) {
			foreach(QDBusObjectPath i, replydevs.value()) {
				devPath = i.path();
				devList.append(getDeviceName(connection,devPath));
			}
		}
		else {
			checkAccessRights(replydevs.error());
		}
		return devList;
	}

	QStringList listEnvironmentNames(QDBusConnection * connection, QString &devPath) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath,*connection, 0);
		QDBusReply<QList<QDBusObjectPath> > replyenvs = dbusDev.getEnvironments();
		QStringList envList;
		if (replyenvs.isValid()) {
			QString envPath;
			int count = 0;
			foreach(QDBusObjectPath i, replyenvs.value()) {
				envPath = i.path();
				envList.append(QString("%1 %2").arg(QString::number(count),getEnvironmentName(connection,envPath)));
				count++;
			}
		}
		else {
			checkAccessRights(replyenvs.error());
			envList = QStringList();
		}
		return envList;
	}

	QString getDeviceName(QDBusConnection * connection, QString &devPath) {
		return getDeviceProperties(connection,devPath).name;
	}
	QString getDeviceType(QDBusConnection * connection, QString &devPath) {
		return toString(getDeviceProperties(connection,devPath).type);
	}
	QString getDeviceState(QDBusConnection * connection, QString &devPath) {
		return toString(getDeviceProperties(connection,devPath).state);
	}
	QString getActiveEnvironment(QDBusConnection * connection, QString &devPath) {
		QString activeEnv = getDeviceProperties(connection,devPath).activeEnvironment;
		if (!activeEnv.isEmpty()) {
			return getEnvironmentName(connection,activeEnv);
		}
		else {
			return QString();
		}
	}
	QString getEnvironmentName(QDBusConnection * connection, QString &envPath) {
		return getEnvironmentProperties(connection,envPath).name;
	}
	QString getEnvironmentState(QDBusConnection * connection, QString &envPath) {
		bool active = getEnvironmentProperties(connection,envPath).active;
		return (active) ? "active" : "deactive";
	}

	bool setEnvironment(QDBusConnection * connection, QString &devPath, int index) {
		if (index < 0)
			return false;
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath, *connection, 0);
		QDBusReply<QList<QDBusObjectPath> > replyenvs = dbusDev.getEnvironments();
		if (replyenvs.isValid()) {
			if (index < replyenvs.value().size() && !replyenvs.value().isEmpty()) {
				dbusDev.setEnvironment(index);
			}
			else {
				return false;
			}
		}
		return false;
	}

	void setEnvironment(QDBusConnection * connection, QString &devPath, QString &envPath) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath, *connection, 0);
		if (!devPath.isEmpty())
			dbusDev.setEnvironment(QDBusObjectPath(envPath));
	}

	void enableDevice(QDBusConnection * connection, QString &devPath) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath, *connection, 0);
		if (!devPath.isEmpty())
			dbusDev.enable();
	}

	void disableDevice(QDBusConnection * connection, QString &devPath) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath, *connection, 0);
		if (!devPath.isEmpty())
			dbusDev.disable();
	}

	//"Private functions"
	QString getDevicePathByName(QDBusConnection * connection, QString & name) {
		DBusDeviceManagerInterface devmgr(NUT_DBUS_URL, "/manager",*connection, 0);
		QDBusReply<QList<QDBusObjectPath> > replydevs = devmgr.getDeviceList();
		if (replydevs.isValid()) {
			//Find device
			QDBusReply<libnutcommon::DeviceProperties> devprops;
			foreach(QDBusObjectPath i, replydevs.value()) {
				DBusDeviceInterface dev(NUT_DBUS_URL, i.path() ,*connection, 0);
				devprops = dev.getProperties();
				if (devprops.isValid()) {
					if (devprops.value().name == name) {
						return i.path();
					}
				}
			}
			return QString();
		}
		else {
			checkAccessRights(replydevs.error());
			return QString();
		}
	}
	
	QString getEnvironmentPathByName(QDBusConnection * connection, QString &devPath, QString &envName) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath, *connection, 0);
		QDBusReply<QList<QDBusObjectPath> > replyenvs = dbusDev.getEnvironments();
		if (replyenvs.isValid()) {
			//Find environment
			QString envPath;
			foreach(QDBusObjectPath i, replyenvs.value()) {
				envPath = i.path();
				if ( getEnvironmentName(connection,envPath) == envName)  {
					break;
				}
			}
			return envPath;
		}
		else {
			return QString();
		}
	}
	QString getEnvironmentPathByIndex(QDBusConnection * connection, QString &devPath, qint32 index) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath,*connection, 0);
		QDBusReply<QList<QDBusObjectPath> > replyenvs = dbusDev.getEnvironments();
		QString envPath;
		if (replyenvs.isValid()) {
			if (index >= 0 && replyenvs.value().size()-1 >= index && replyenvs.value().size() > 0) {
				envPath = replyenvs.value()[index].path();
			}
		}
		else {
			checkAccessRights(replyenvs.error());
		}
		return envPath;
	}
	
	libnutcommon::DeviceProperties getDeviceProperties(QDBusConnection * connection, QString &devPath) {
		DBusDeviceInterface dbusDev(NUT_DBUS_URL, devPath ,*connection, 0);
		QDBusReply<libnutcommon::DeviceProperties> replyprops = dbusDev.getProperties();
		if (replyprops.isValid()) {
			return replyprops.value();
		}
		else {
			return libnutcommon::DeviceProperties();
		}
	}
	libnutcommon::EnvironmentProperties getEnvironmentProperties(QDBusConnection * connection, QString &env) {
		DBusEnvironmentInterface dbusEnv(NUT_DBUS_URL,env,*connection,0);
		QDBusReply<libnutcommon::EnvironmentProperties> replyprops = dbusEnv.getProperties();
		if (replyprops.isValid()) {
			return replyprops.value();
		}
		else {
			return libnutcommon::EnvironmentProperties();
		}
	}
	void checkAccessRights(QDBusError error) {
		if (QDBusError::AccessDenied == error.type()) {
			print(QString("AccessDenied"));
		}
	}
}
