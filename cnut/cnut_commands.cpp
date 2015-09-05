#include "cnut_commands.h"
#include <libnutclientbase/dbus.h>

#include <QTextStream>

namespace cnut {
	using namespace libnutclientbase;

	namespace {
		void checkAccessRights(QDBusError error) {
			QTextStream out { stderr };
			if (QDBusError::AccessDenied == error.type()) {
				out << QString("AccessDenied\n");
			}
			else if (QDBusError::InvalidSignature == error.type()) {
				out << QString("AccessDenied(InvalidSignature)\n");
			}
			else {
				out << QDBusError::errorString(error.type()) << "\n";
			}
		}

		inline void waitForFinished(QDBusPendingCall call) {
			call.waitForFinished();
		}

		template<typename... Result>
		void waitForFinished(QDBusReply<Result...> const& reply) {
			// already finished
		}

		template<typename Reply>
		auto filterDbusResult(Reply&& reply) -> decltype(reply.value()) {
			waitForFinished(reply); // should already be finished, but make sure
			if (!reply.isValid()) {
				checkAccessRights(reply.error());
				return { };
			}
			return reply.value();
		}
	}


	//"Public functions"
	QStringList listDeviceNames(QDBusConnection& connection) {
		DBusDeviceManager devmgr(connection);
		return filterDbusResult(devmgr.sync_getDeviceNames());
	}

	QStringList listEnvironmentNames(QDBusConnection& connection, QDBusObjectPath const& devPath, bool withIndex) {
		DBusDevice dbusDev(devPath, connection);
		auto envPaths = filterDbusResult(dbusDev.sync_getEnvironments());
		QStringList envList;
		for (auto const& envPath: envPaths) {
			DBusEnvironment dbusEnv(envPath, connection);
			auto envName = filterDbusResult(dbusEnv.sync_getName());
			if (withIndex) {
				envList << QString("%1 %2").arg(QString::number(envList.size()), envName);
			}
			else {
				envList << (envName.isEmpty() ? QString::number(envList.size()) : envName);
			}
		}
		return envList;
	}

	QStringList listInterfaceIndexes(QDBusConnection& connection, QDBusObjectPath const& envPath) {
		DBusEnvironment dbusEnv(envPath, connection);
		QStringList ifList;
		for (auto const& ifNdx: filterDbusResult(dbusEnv.sync_getInterfaceIds())) {
			ifList << QString::number(ifNdx);
		}
		return ifList;
	}

	QString getDeviceType(QDBusConnection& connection, QDBusObjectPath const& devPath) {
		DBusDevice dbusDev(devPath, connection);
		return toString(filterDbusResult(dbusDev.sync_getType()));
	}
	QString getDeviceState(QDBusConnection& connection, QDBusObjectPath const& devPath) {
		DBusDevice dbusDev(devPath, connection);
		return toString(filterDbusResult(dbusDev.sync_getState()));
	}
	QString getActiveEnvironment(QDBusConnection& connection, QDBusObjectPath const& devPath) {
		DBusDevice dbusDev(devPath, connection);
		auto activeEnv = filterDbusResult(dbusDev.sync_getActiveEnvironment());
		if (activeEnv) {
			DBusEnvironment dbusEnv(activeEnv.objectPath(), connection);
			return filterDbusResult(dbusEnv.sync_getName());
		}
		return { };
	}
	QString getEnvironmentActive(QDBusConnection& connection, QDBusObjectPath const& envPath) {
		DBusEnvironment dbusEnv(envPath, connection);
		return filterDbusResult(dbusEnv.sync_isActive()) ? "active" : "inactive";
	}
	QString getEnvironmentSelectable(QDBusConnection& connection, QDBusObjectPath const& envPath) {
		DBusEnvironment dbusEnv(envPath, connection);
		auto reply = dbusEnv.sync_getSelectResult();
		if (!reply.isValid()) {
			checkAccessRights(reply.error());
			return {};
		}

		switch (reply.value()) {
		case libnutcommon::SelectResult::False:
			return "no";
		case libnutcommon::SelectResult::True:
			return "selected";
		case libnutcommon::SelectResult::User:
			return "yes";
		default:
			return "unknown";
		}
	}
	QString getInterfaceState(QDBusConnection& connection, QDBusObjectPath const& ifPath) {
		DBusInterface_IPv4 dbusIf(ifPath, connection);
		return toString(filterDbusResult(dbusIf.sync_getState()));
	}
	QStringList getInterfaceProperties(QDBusConnection& connection, QDBusObjectPath const& ifPath) {
		DBusInterface_IPv4 dbusIf(ifPath, connection);
		auto propsReply = dbusIf.sync_getProperties();
		if (!propsReply.isValid()) {
			checkAccessRights(propsReply.error());
			return { };
		}

		auto props = propsReply.value();
		QStringList propList = {
			props.ip.toString(),
			props.netmask.toString(),
			props.gateway.toString(),
		};
		for (auto const& dns: props.dnsServers) {
			propList << dns.toString();
		}
		return propList;
	}

	void setEnvironment(QDBusConnection& connection, QDBusObjectPath const& devPath, qint32 index) {
		DBusDevice dbusDev(devPath, connection);
		dbusDev.setEnvironment(index);
	}

	void setEnvironment(QDBusConnection& connection, QDBusObjectPath const& devPath, QDBusObjectPath const& envPath) {
		DBusDevice dbusDev(devPath, connection);
		dbusDev.setEnvironment(envPath);
	}

	void enableDevice(QDBusConnection& connection, QDBusObjectPath const& devPath) {
		DBusDevice dbusDev(devPath, connection);
		dbusDev.enable();
	}

	void disableDevice(QDBusConnection& connection, QDBusObjectPath const& devPath) {
		DBusDevice dbusDev(devPath, connection);
		dbusDev.disable();
	}

	OptionalQDBusObjectPath getDevicePathByName(QDBusConnection& connection, QString const& devName) {
		auto devPath = DBusDevice::makePath(devName);
		DBusDevice dbusDev(devPath, connection);
		if (dbusDev.sync_getState().isValid()) return devPath;
		return { };
	}

	OptionalQDBusObjectPath getEnvironmentPathByName(QDBusConnection& connection, QDBusObjectPath const& devPath, QString const& envName) {
		DBusDevice dbusDev(devPath, connection);
		auto result = OptionalQDBusObjectPath{};
		for (auto const& envPath: filterDbusResult(dbusDev.sync_getEnvironments())) {
			DBusEnvironment dbusEnv(envPath, connection);
			auto replyName = dbusEnv.sync_getName();
			if (replyName.isValid() && envName == replyName.value()) {
				if (result) {
					QTextStream(stderr) << "Environment name not unique, use index instead\n";
					return { };
				}
				result = envPath;
			}
		}
		return result;
	}

	OptionalQDBusObjectPath getEnvironmentPathByIndex(QDBusConnection& connection, QDBusObjectPath const& devPath, qint32 envNdx) {
		auto envPath = DBusEnvironment::makePath(devPath, envNdx);
		DBusEnvironment dbusEnv(envPath, connection);
		if (dbusEnv.sync_isActive().isValid()) return envPath;
		return { };
	}

	OptionalQDBusObjectPath getInterfacePathByIndex(QDBusConnection& connection, QDBusObjectPath const& envPath, qint32 ifNdx) {
		auto ifPath = DBusInterface_IPv4::makePath(envPath, ifNdx);
		DBusInterface_IPv4 dbusIf(ifPath, connection);
		if (dbusIf.sync_getState().isValid()) return ifPath;
		return { };
	}
}
