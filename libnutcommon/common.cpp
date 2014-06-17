#include "common.h"

QDBusArgument &operator<< (QDBusArgument &argument, const QHostAddress &data) {
	argument.beginStructure();
	argument << data.toString();
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>> (const QDBusArgument &argument, QHostAddress &data) {
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

	void init() {
		static int done = 0;
		if (done) return;
		done = 1;

		config_init();
		device_init();

		qRegisterMetaType< QHostAddress >("QHostAddress");
		qRegisterMetaType< QList<QHostAddress> >("QList<QHostAddress>");
		qDBusRegisterMetaType< QHostAddress >();
		qDBusRegisterMetaType< QList<QHostAddress> >();

		qRegisterMetaType< QVector<quint32> >("QVector<quint32>");
		qRegisterMetaType< QVector< QVector<quint32> > >("QVector< QVector<quint32> >");
		qDBusRegisterMetaType< QVector<quint32> >();
		qDBusRegisterMetaType< QVector< QVector<quint32> > >();

		qRegisterMetaType< QList<qint32> >("QList<qint32>");
		qDBusRegisterMetaType< QList<qint32> >();
	}
}
