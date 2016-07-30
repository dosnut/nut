#include "common.h"

QDBusArgument& operator<<(QDBusArgument& argument, QHostAddress const& data) {
	argument.beginStructure();
	argument << data.toString();
	argument.endStructure();
	return argument;
}
QDBusArgument const& operator>>(QDBusArgument const& argument, QHostAddress& data) {
	argument.beginStructure();
	QString addr;
	argument >> addr;
	data = QHostAddress(addr);
	argument.endStructure();
	return argument;
}

namespace libnutcommon {
	void config_init();
	void device_init();
	void macaddress_init();
	void ssid_init();

	void init() {
		static int done = 0;
		if (done) return;
		done = 1;

		config_init();
		device_init();
		macaddress_init();
		ssid_init();

		qRegisterMetaType<QHostAddress>("QHostAddress");
		qRegisterMetaType<QList<QHostAddress>>("QList<QHostAddress>");
		qDBusRegisterMetaType<QHostAddress>();
		qDBusRegisterMetaType<QList<QHostAddress>>();

		qRegisterMetaType<QVector<quint32>>("QVector<quint32>");
		qRegisterMetaType<QVector<QVector<quint32>>>("QVector< QVector<quint32> >");
		qDBusRegisterMetaType<QVector<quint32>>();
		qDBusRegisterMetaType<QVector<QVector<quint32>>>();

		qRegisterMetaType<QList<qint32>>("QList<qint32>");
		qDBusRegisterMetaType<QList<qint32>>();
	}
}
