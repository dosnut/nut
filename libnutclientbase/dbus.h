#ifndef LIBNUTCLIENTBASE_DBUS_H
#define LIBNUTCLIENTBASE_DBUS_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QtDBus>
#include "libnutcommon/common.h"

#include <functional>
#include <memory>

/*
 * ## Generic interface for methods with return values:
 *
 * It is recommended to use the callback based methods or the synchronous
 * method.
 *
 * The others are just part of the implemention.
 *
 * - (optional) QDBusPendingCallWatcher* cached_METHOD(...)
 *   If the result of one call can be used for multiple requests a interface
 *   may provide cached_METHOD; if no call is already pending this will create
 *   a new one, otherwise it returns the pending one.
 * - QDBusPendingReply<ResultType> async_METHOD(...)
 *   Starts a call (or uses an already pending one), but doesn't wait for it
 * - QDBusPendingReply<ResultType> sync_METHOD(...)
 *   Same as async_METHOD, but waits for the result before returning
 * - METHOD(..., returnMethod, [errorMethod], [parentobj])
 *   starts asynchronous call; calls returnMethod on success with response,
 *   errorMethod with error on failure; if parentobj is not null the callbacks
 *   are not called if parentobj gets deleted.
 *   if errorMethod is not given errors are ignored.
 *
 * Methods with no response (Q_NOREPLY) are just simple methods, but are also
 * available as slots.
 * Some methods can optionally be available without reply (and as slots).
 *
 * Passing responses is not done with signals for two reasons:
 * - if you forget to disconnect you might get responses without having asked
 *   for them; although this might be useful sometimes it is bad design.
 * - when connecting to a signal of QDBusAbstractInterface Qt tries to connect
 *   to the signal of the remote end - but such response signals should be
 *   local only.
 *
 * Also C++-11 makes writing lambdas super easy, so this shouldn't be a
 * problem.
 */

/* Implementation details: use the same macro invocation to declare and
 * implement methods here and in dbus.cpp; signals are connected automatically
 * on demand by QDBusAbstractInterface, no implementation is needed for them
 * here.
 */

/* cacheable methods usually don't take parameters */
#define DBUS_CACHEABLE_METHOD(result, method) \
private: \
	internal::DBusPendingWatcher pending_##method; \
public: \
	typedef result Result_##method; \
	QDBusPendingCallWatcher* cached_##method(); \
	QDBusPendingReply<Result_##method> async_##method(); \
	QDBusReply<Result_##method> sync_##method(); \
	void method(ResultHandler<Result_##method> returnMethod, QObject* parent = 0); \
	void method(ResultHandler<Result_##method> returnMethod, DBusErrorHandler errorMethod, QObject* parent = 0);

/* not cacheable method call with result and 0 parameters */
#define DBUS_ACTION_0(result, method) \
	typedef result Result_##method; \
	QDBusPendingReply<Result_##method> async_##method(); \
	QDBusReply<Result_##method> sync_##method(); \
	void method(ResultHandler<Result_##method> returnMethod, QObject* parent = 0); \
	void method(ResultHandler<Result_##method> returnMethod, DBusErrorHandler errorMethod, QObject* parent = 0);

/* not cacheable method call with result and 1 parameter */
#define DBUS_ACTION_1(result, method, type1) \
	typedef result Result_##method; \
	QDBusPendingReply<Result_##method> async_##method(type1 par1); \
	QDBusReply<Result_##method> sync_##method(type1 par1); \
	void method(type1 par1, ResultHandler<Result_##method> returnMethod, QObject* parent = 0); \
	void method(type1 par1, ResultHandler<Result_##method> returnMethod, DBusErrorHandler errorMethod, QObject* parent = 0);

/* not cacheable method call without result and 0 parameters */
#define DBUS_NOREPLY_ACTION_0(method) \
public slots: \
	void method(); \
public:

/* not cacheable method call without result and 1 parameter */
#define DBUS_NOREPLY_ACTION_1(method, type1) \
public slots: \
	void method(type1 par1); \
public:

namespace libnutclientbase {
	/* callback type for functions returning Result */
	template<typename... Result>
	using ResultHandler = std::function<void(Result...)>;

	/* callback type for error handling */
	using DBusErrorHandler = ResultHandler<QDBusError>;

	namespace internal {
		/* base class to connect QDBusPendingCallWatcher finished() signal with;
		 * instances are templates and can't have own slots/signals (Qt limitation)
		 */
		class DBusCallbackWatcherBase : public QObject {
			Q_OBJECT
		protected:
			explicit DBusCallbackWatcherBase(QObject* parent, QDBusPendingCallWatcher* watcher);

		private slots:
			void slot_finished(QDBusPendingCallWatcher* watcher);

		protected:
			virtual void finished(QDBusPendingCallWatcher* watcher) = 0;
		};

		/* class to track pending calls and cleanup when they are finished */
		class DBusPendingWatcher : public QObject {
			Q_OBJECT
		public:
			explicit DBusPendingWatcher(QObject* parent = nullptr);

			std::unique_ptr<QDBusPendingCallWatcher> pending;

		public slots:
			void finished();
		};
	}


	class DBusDeviceManager: public QDBusAbstractInterface {
		Q_OBJECT
	private:
		static const char* InterfaceName;
		static const char* DefaultPath;

	public:
		static QDBusObjectPath makePath();

		explicit DBusDeviceManager(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		/** use connection and default service/path */
		explicit DBusDeviceManager(QDBusConnection connection, QObject* parent = 0);
		/** use default path, and connection and service from `from` */
		explicit DBusDeviceManager(QDBusAbstractInterface* from, QObject* parent = 0);

		DBUS_ACTION_0(QList<QDBusObjectPath>, getDeviceList)
		DBUS_ACTION_0(QStringList, getDeviceNames)

	signals:
		void deviceAddedPath(const QDBusObjectPath &objectpath);
		void deviceRemovedPath(const QDBusObjectPath &objectpath);
		void deviceAddedName(const QString &devname);
		void deviceRemovedName(const QString &devname);
	};


	class DBusDevice: public QDBusAbstractInterface {
		Q_OBJECT
	public:
		static const char* InterfaceName;
		static const char* BasePath;

	public:
		static QDBusObjectPath makePath(QString devName);

		explicit DBusDevice(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		explicit DBusDevice(QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		explicit DBusDevice(QDBusObjectPath path, QDBusAbstractInterface* from, QObject* parent = 0);

		DBUS_ACTION_0(libnutcommon::DeviceProperties, getProperties)
		/* single properties */
		/* constant properties */
		DBUS_CACHEABLE_METHOD(QString, getName)
		/* variable properties */
		DBUS_ACTION_0(libnutcommon::DeviceType, getType)
		DBUS_ACTION_0(libnutcommon::OptionalQDBusObjectPath, getActiveEnvironmentPath)
		DBUS_ACTION_0(qint32, getActiveEnvironmentIndex)
		DBUS_ACTION_0(libnutcommon::DeviceState, getState)
		DBUS_ACTION_0(QString, getEssid)
		DBUS_ACTION_0(libnutcommon::MacAddress, getMacAddress)

		/* constant config */
		DBUS_CACHEABLE_METHOD(libnutcommon::DeviceConfig, getConfig)

		/* list of sub objects (list doesn't change) */
		DBUS_CACHEABLE_METHOD(QList<QDBusObjectPath>, getEnvironments)
		DBUS_CACHEABLE_METHOD(QList<qint32>, getEnvironmentIds)

		/* actions */
		DBUS_NOREPLY_ACTION_0(enable)
		DBUS_NOREPLY_ACTION_0(disable)
		DBUS_NOREPLY_ACTION_1(setEnvironment, QDBusObjectPath)
		DBUS_NOREPLY_ACTION_1(setEnvironment, qint32)

	signals:
		void propertiesChanged(libnutcommon::DeviceProperties properties);
		void stateChanged(libnutcommon::DeviceState state);
		void activeEnvironmentChangedPath(libnutcommon::OptionalQDBusObjectPath objectpath);
		void activeEnvironmentChangedIndex(qint32 envId);
		void nextEnvironmentChangedPath(libnutcommon::OptionalQDBusObjectPath objectpath);
		void nextEnvironmentChangedIndex(qint32 envId);
		void userPreferredEnvironmentChangedPath(libnutcommon::OptionalQDBusObjectPath objectpath);
		void userPreferredEnvironmentChangedIndex(qint32 envId);
		void newWirelessNetworkFound();
	};


	class DBusEnvironment: public QDBusAbstractInterface {
		Q_OBJECT
	private:
		static const char* InterfaceName;

	public:
		static QDBusObjectPath makePath(QString devName, qint32 envNdx);
		static QDBusObjectPath makePath(QDBusObjectPath devPath, qint32 envNdx);

		explicit DBusEnvironment(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		explicit DBusEnvironment(QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		explicit DBusEnvironment(QDBusObjectPath path, QDBusAbstractInterface* from, QObject* parent = 0);

		DBUS_ACTION_0(libnutcommon::EnvironmentProperties, getProperties)
		/* single properties */
		/* constant properties */
		DBUS_CACHEABLE_METHOD(QString, getName)
		DBUS_CACHEABLE_METHOD(qint32, getID)
		/* variable properties */
		DBUS_ACTION_0(bool, isActive)
		DBUS_ACTION_0(libnutcommon::SelectResult, getSelectResult)
		DBUS_ACTION_0(QVector<libnutcommon::SelectResult>, getSelectResults)

		/* constant config */
		DBUS_CACHEABLE_METHOD(libnutcommon::EnvironmentConfig, getConfig)

		/* list of sub objects (list doesn't change) */
		DBUS_CACHEABLE_METHOD(QList<QDBusObjectPath>, getInterfaces)
		DBUS_CACHEABLE_METHOD(QList<qint32>, getInterfaceIds)

	signals:
		void propertiesChanged(libnutcommon::EnvironmentProperties properties);
	};


	class DBusInterface_IPv4: public QDBusAbstractInterface {
		Q_OBJECT
	private:
		static const char* InterfaceName;

	public:
		static QDBusObjectPath makePath(QString devName, qint32 envNdx, qint32 ifNdx);
		static QDBusObjectPath makePath(QDBusObjectPath devPath, qint32 envNdx, qint32 ifNdx);
		static QDBusObjectPath makePath(QDBusObjectPath envPath, qint32 ifNdx);

		explicit DBusInterface_IPv4(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		explicit DBusInterface_IPv4(QDBusObjectPath path, QDBusConnection connection, QObject* parent = 0);
		explicit DBusInterface_IPv4(QDBusObjectPath path, QDBusAbstractInterface* from, QObject* parent = 0);

		DBUS_ACTION_0(libnutcommon::InterfaceProperties, getProperties)
		/* single properties */
		/* variable properties */
		DBUS_ACTION_0(libnutcommon::InterfaceState, getState)
		DBUS_ACTION_0(QHostAddress, getIP)
		DBUS_ACTION_0(QHostAddress, getNetmask)
		DBUS_ACTION_0(QHostAddress, getGateway)
		DBUS_ACTION_0(QList<QHostAddress>, getDnsServers)
		DBUS_ACTION_0(int, getGatewayMetric)
		DBUS_ACTION_0(bool, needUserSetup)

		/* constant config */
		DBUS_CACHEABLE_METHOD(libnutcommon::IPv4Config, getConfig)

		/* variable "user" config */
		DBUS_ACTION_0(libnutcommon::IPv4UserConfig, getUserConfig)

		/* actions */
		DBUS_ACTION_1(bool, setUserConfig, libnutcommon::IPv4UserConfig)
		DBUS_NOREPLY_ACTION_1(setUserConfig, libnutcommon::IPv4UserConfig)

	signals:
		void propertiesChanged(libnutcommon::InterfaceProperties properties);
		void userConfigChanged(libnutcommon::IPv4UserConfig userConfig);
	};
}

#undef DBUS_CACHEABLE_METHOD
#undef DBUS_ACTION_0
#undef DBUS_ACTION_1
#undef DBUS_NOREPLY_ACTION_0
#undef DBUS_NOREPLY_ACTION_1

#endif
