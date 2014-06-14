#ifndef NUT_COMMON_DEVICE_H
#define NUT_COMMON_DEVICE_H

#include <QString>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>
#include <QList>

#include "macaddress.h"

namespace libnutcommon {
	class OptionalQDBusObjectPath {
	private:
		QString m_path;

	public:
		inline OptionalQDBusObjectPath() : m_path() { }
		explicit inline OptionalQDBusObjectPath(QDBusObjectPath path) : m_path(path.path()) { }
		explicit inline OptionalQDBusObjectPath(QString path) : m_path(path) { }

		inline friend bool operator==(const OptionalQDBusObjectPath& a, const OptionalQDBusObjectPath& b) { return a.m_path == b.m_path; }
		inline friend bool operator!=(const OptionalQDBusObjectPath& a, const OptionalQDBusObjectPath& b) { return a.m_path != b.m_path; }

		inline const QString& path() const { return m_path; }
		inline QDBusObjectPath objectPath() const {
			return QDBusObjectPath(m_path);
		}
		inline operator bool() const { return !m_path.isEmpty(); }
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const OptionalQDBusObjectPath &optionalObjPath);
	const QDBusArgument &operator>> (const QDBusArgument &argument, OptionalQDBusObjectPath &optionalObjPath);

	enum class DeviceState : quint32 {
		DEACTIVATED = 0,
		ACTIVATED,
		CARRIER,
		UNCONFIGURED,
		UP,
	};
	QString toString(DeviceState state);
	QDBusArgument &operator<< (QDBusArgument &argument, DeviceState state);
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceState &state);

	enum class DeviceType : quint32 {
		ETH = 0,
		AIR,
		PPP,
		BRIDGE,
	};
	QString toString(DeviceType type);
	QDBusArgument &operator<< (QDBusArgument &argument, DeviceType type);
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceType &type);

	enum class InterfaceState : quint32 {
		OFF = 0,
		STATIC,
		DHCP,
		ZEROCONF,
		WAITFORCONFIG,
	};
	QString toString(InterfaceState state);
	QDBusArgument &operator<< (QDBusArgument &argument, InterfaceState state);
	const QDBusArgument &operator>> (const QDBusArgument &argument, InterfaceState &state);

	struct DeviceProperties {
		QString name;
		OptionalQDBusObjectPath activeEnvironment;
		DeviceState state = DeviceState::DEACTIVATED;
		DeviceType type = DeviceType::ETH;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceProperties &devprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceProperties &devprop);

	struct EnvironmentProperties {
		QString name;
		bool active = false;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentProperties &envprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentProperties &envprop);

	struct InterfaceProperties {
		InterfaceState ifState = InterfaceState::OFF;
		QHostAddress ip;
		QHostAddress netmask;
		QHostAddress gateway;
		QList<QHostAddress> dns;
		int gateway_metric = -1;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const InterfaceProperties &ifprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, InterfaceProperties &ifprop);
}

Q_DECLARE_METATYPE(libnutcommon::OptionalQDBusObjectPath)
Q_DECLARE_METATYPE(libnutcommon::DeviceState)
Q_DECLARE_METATYPE(libnutcommon::DeviceType)
Q_DECLARE_METATYPE(libnutcommon::InterfaceState)
Q_DECLARE_METATYPE(libnutcommon::DeviceProperties)
Q_DECLARE_METATYPE(libnutcommon::EnvironmentProperties)
Q_DECLARE_METATYPE(libnutcommon::InterfaceProperties)

#endif
