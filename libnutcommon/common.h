/*
 * Includes all common stuff, needed by server and client.
 *
 */

#ifndef NUT_COMMON_COMMON_H
#define NUT_COMMON_COMMON_H

// If you change this, change it in nuts/dbus.h too (as moc does not expand macros).
#define NUT_DBUS_URL "de.unistuttgart.nut"

#include <QString>
#include <QHostAddress>
#include <QtDBus>
#include <QDBusArgument>
#include <QMetaType>
#include <QHash>

#include <type_traits>

namespace libnutcommon {
	/** Init the libnutcommon library; you may call it more than once.
	 */
	void init();
}

#if QT_VERSION < 0x040500
static inline bool operator== (const QDBusObjectPath &p1, const QDBusObjectPath &p2){
	return (p1.path() == p2.path());
}

static inline uint qHash(const QDBusObjectPath &key) {
	return qHash(key.path());
}
#endif

QDBusArgument &operator<< (QDBusArgument &argument, const QHostAddress &data);
const QDBusArgument &operator>> (const QDBusArgument &argument, QHostAddress &data);

Q_DECLARE_METATYPE(QHostAddress)
Q_DECLARE_METATYPE(QList<QHostAddress>)

Q_DECLARE_METATYPE(QVector<quint32>)
Q_DECLARE_METATYPE(QVector< QVector<quint32> >)

Q_DECLARE_METATYPE(QList<qint32>)

#include "device.h"
#include "dbusmanager.h"
#include "version.h"

#endif
