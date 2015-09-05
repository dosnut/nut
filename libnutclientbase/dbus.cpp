#include "dbus.h"

#include <QVariant>
#include <QMetaType>

/* #define CLASS to current class before using macros below;
 * use the same definitions as in dbus.h (that is why the macros have
 * sometimes an unused resultType parameter).
 */

/* cacheable methods have no parameter but a result type,
 * which is required to be typedef'ed to Result_##method
 */
#define DBUS_CACHEABLE_METHOD(resultType, method) \
	QDBusPendingCallWatcher* CLASS::cached_##method() { \
		return dbusGetCachedWatcher(this, pending_##method, [this]() { \
			static auto const methodName = QLatin1String(#method); \
			return asyncCall(methodName); \
		}); \
	} \
	auto CLASS::async_##method() -> QDBusPendingReply<Result_##method> { \
		return *cached_##method(); \
	} \
	auto CLASS::sync_##method() -> QDBusReply<Result_##method> { \
		return async_##method(); \
	} \
	void CLASS::method(ResultHandler<Result_##method> returnMethod, QObject *parent) { \
		dbusCallbackWatch(cached_##method(), parent, std::move(returnMethod)); \
	} \
	void CLASS::method(ResultHandler<Result_##method> returnMethod, DBusErrorHandler errorMethod, QObject *parent) { \
		dbusCallbackWatch(cached_##method(), parent, std::move(returnMethod), std::move(errorMethod)); \
	}

/* action method with 0 parameters and a result type,
 * which is required to be typedef'ed to Result_##method
 */
#define DBUS_ACTION_0(resultType, method) \
	auto CLASS::async_##method() -> QDBusPendingReply<Result_##method> { \
		static auto const methodName = QLatin1String(#method); \
		return asyncCall(methodName); \
	} \
	auto CLASS::sync_##method() -> QDBusReply<Result_##method> { \
		return async_##method(); \
	} \
	void CLASS::method(ResultHandler<Result_##method> returnMethod, QObject *parent) { \
		dbusCallbackWatch(this, async_##method(), parent, std::move(returnMethod)); \
	} \
	void CLASS::method(ResultHandler<Result_##method> returnMethod, DBusErrorHandler errorMethod, QObject *parent) { \
		dbusCallbackWatch(this, async_##method(), parent, std::move(returnMethod), std::move(errorMethod)); \
	}

/* action method with 1 parameter and a result type,
 * which is required to be typedef'ed to Result_##method
 */
#define DBUS_ACTION_1(resultType, method, type1) \
	auto CLASS::async_##method(type1 par1) -> QDBusPendingReply<Result_##method> { \
		static auto const methodName = QLatin1String(#method); \
		return asyncCall(methodName, QVariant::fromValue(std::move(par1))); \
	} \
	auto CLASS::sync_##method(type1 par1) -> QDBusReply<Result_##method> { \
		return async_##method(std::move(par1)); \
	} \
	void CLASS::method(type1 par1, ResultHandler<Result_##method> returnMethod, QObject *parent) { \
		dbusCallbackWatch(this, async_##method(std::move(par1)), parent, std::move(returnMethod)); \
	} \
	void CLASS::method(type1 par1, ResultHandler<Result_##method> returnMethod, DBusErrorHandler errorMethod, QObject *parent) { \
		dbusCallbackWatch(this, async_##method(std::move(par1)), parent, std::move(returnMethod), std::move(errorMethod)); \
	}

/* noreply actions without a parameter */
#define DBUS_NOREPLY_ACTION_0(method) \
	void CLASS::method() { \
		static auto const methodName = QLatin1String(#method); \
		call(QDBus::NoBlock, methodName); \
	}

/* noreply actions with one parameter */
#define DBUS_NOREPLY_ACTION_1(method, type1) \
	void CLASS::method(type1 par1) { \
		static auto const methodName = QLatin1String(#method); \
		call(QDBus::NoBlock, methodName, QVariant::fromValue(par1)); \
	}


namespace libnutclientbase {
	namespace internal {
		DBusCallbackWatcherBase::DBusCallbackWatcherBase(QObject* parent, QDBusPendingCallWatcher* watcher)
		: QObject(nullptr != parent ? parent : watcher) {
			connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(slot_finished(QDBusPendingCallWatcher*)));
		}

		void DBusCallbackWatcherBase::slot_finished(QDBusPendingCallWatcher* watcher) {
			/* delete itself and the watcher; the watcher could be a temporary
			 * owned by a QDBusAbstractInterface and wouldn't get deleted
			 * otherwise
			 */
			deleteLater();
			watcher->deleteLater();
			finished(watcher);
		}

		DBusPendingWatcher::DBusPendingWatcher(QObject* parent)
		: QObject(parent) {
		}

		void DBusPendingWatcher::finished() {
			/* release pending before any other slot is called, as this watcher is
			 * the first that got connected.
			 */
			auto watcher = pending.release();
			if (watcher) watcher->deleteLater();
		}
	}

	using namespace internal;

	namespace {
		auto ignoreError = DBusErrorHandler{ [](QDBusError) { } };

		/* goal: call returnMethod(result.argumentAt<0>(), result.argumentAt<1>(), ...) */
		template<typename... Result, typename ReturnMethod>
		void dbusCallSuccessCallback(QDBusPendingReply<Result...>& result, ReturnMethod&& returnMethod);

		/* implement for one result: we don't actually use result tuples, although the rest of the API might support it */
		template<typename Result, typename ReturnMethod>
		void dbusCallSuccessCallback(QDBusPendingReply<Result>& result, ReturnMethod&& returnMethod) {
			returnMethod(result.template argumentAt<0>());
		}

		/* call returnMethod with result or errorMethod with error. waits to make sure it is either an error or a result. */
		template<typename... Result, typename ReturnMethod, typename ErrorMethod>
		void dbusCallCallback(QDBusPendingCallWatcher *watcher, ReturnMethod&& returnMethod, ErrorMethod&& errorMethod) {
			watcher->waitForFinished();
			QDBusPendingReply<Result...> result{*watcher};
			if (result.isError()) {
				errorMethod(result.error());
			} else {
				dbusCallSuccessCallback(result, std::forward<ReturnMethod>(returnMethod));
			}
		}

		/* wait asynchronously for result and call callbacks; base class does the connect()
		 * and also marks this and the watcher for deletion after it finished
		 */
		template<typename... Result>
		class DBusCallbackWatcher : public DBusCallbackWatcherBase {
		public:
			ResultHandler<Result...> returnMethod;
			DBusErrorHandler errorMethod;

			DBusCallbackWatcher(QObject *parent, QDBusPendingCallWatcher *watcher, ResultHandler<Result...> returnMethod, DBusErrorHandler errorMethod)
			: DBusCallbackWatcherBase(parent, watcher), returnMethod(std::move(returnMethod)), errorMethod(std::move(errorMethod)) {
			}

			void finished(QDBusPendingCallWatcher* watcher) override {
				dbusCallCallback<Result...>(watcher, returnMethod, errorMethod);
			}
		};

		/* after creating a watcher (which must be alive for the duration of the request) */
		template<typename... Result>
		void dbusCallbackWatch(QDBusPendingCallWatcher *watcher, QObject *parent, ResultHandler<Result...> returnMethod, DBusErrorHandler errorMethod = ignoreError) {
			new DBusCallbackWatcher<Result...>(parent, watcher, std::move(returnMethod), std::move(errorMethod));
		}

		/* create a new watcher which gets automatically destroyed after the request is done */
		template<typename... Result>
		void dbusCallbackWatch(QDBusAbstractInterface *i, QDBusPendingCall call, QObject *parent, ResultHandler<Result...> returnMethod, DBusErrorHandler errorMethod = ignoreError) {
			/* DBusCallbackWatcherBase::slot_finished will clean this up for us */
			auto watcher = new QDBusPendingCallWatcher(call, i);
			dbusCallbackWatch(watcher, parent, std::move(returnMethod), std::move(errorMethod));
		}

		/* get a cached or create a new watcher with makeCall, make sure the pending call gets cleaned up when the result comes in */
		template<typename MakeCall>
		QDBusPendingCallWatcher* dbusGetCachedWatcher(QDBusAbstractInterface *i, DBusPendingWatcher& container, MakeCall&& makeCall) {
			auto watcher = container.pending.get();
			if (nullptr == watcher) {
				container.pending.reset(watcher = new QDBusPendingCallWatcher(makeCall(), &container));
				QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &container, SLOT(finished()));
			}
			return watcher;
		}
	}

	const char* DBusDeviceManager::InterfaceName = NUT_DBUS_URL ".DeviceManager";
	const char* DBusDeviceManager::DefaultPath = "/manager";

	QDBusObjectPath DBusDeviceManager::makePath() {
		return QDBusObjectPath{ DefaultPath };
	}

	DBusDeviceManager::DBusDeviceManager(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: QDBusAbstractInterface(std::move(service), std::move(path).path(), InterfaceName, std::move(connection), parent) {
	}
	DBusDeviceManager::DBusDeviceManager(QDBusConnection connection, QObject* parent)
	: DBusDeviceManager(QString(NUT_DBUS_URL), QDBusObjectPath(DefaultPath), connection, parent) {
	}
	DBusDeviceManager::DBusDeviceManager(QDBusAbstractInterface* from, QObject* parent)
	: DBusDeviceManager(from->service(), QDBusObjectPath(DefaultPath), from->connection(), parent) {
	}

	#define CLASS DBusDeviceManager
		DBUS_ACTION_0(QList<QDBusObjectPath>, getDeviceList)
		DBUS_ACTION_0(QStringList, getDeviceNames)
	#undef CLASS


	const char* DBusDevice::InterfaceName = NUT_DBUS_URL ".Device";
	const char* DBusDevice::BasePath = "/devices/";

	QDBusObjectPath DBusDevice::makePath(QString devName) {
		return QDBusObjectPath{ BasePath + devName };
	}

	DBusDevice::DBusDevice(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: QDBusAbstractInterface(std::move(service), std::move(path).path(), InterfaceName, std::move(connection), parent) {
	}
	DBusDevice::DBusDevice(QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: DBusDevice(QString(NUT_DBUS_URL), std::move(path), std::move(connection), parent) {
	}
	DBusDevice::DBusDevice(QDBusObjectPath path, QDBusAbstractInterface* from, QObject* parent)
	: DBusDevice(from->service(), std::move(path), from->connection(), parent) {
	}

	#define CLASS DBusDevice
		DBUS_ACTION_0(libnutcommon::DeviceProperties, getProperties)
		/* single properties */
		/* constant properties */
		DBUS_CACHEABLE_METHOD(QString, getName)
		/* variable properties */
		DBUS_ACTION_0(libnutcommon::DeviceType, getType)
		DBUS_ACTION_0(libnutcommon::OptionalQDBusObjectPath, getActiveEnvironment)
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
	#undef CLASS


	const char* DBusEnvironment::InterfaceName = NUT_DBUS_URL ".Environment";

	QDBusObjectPath DBusEnvironment::makePath(QString devName, qint32 envNdx) {
		return makePath(DBusDevice::makePath(std::move(devName)), envNdx);
	}
	QDBusObjectPath DBusEnvironment::makePath(QDBusObjectPath devPath, qint32 envNdx) {
		return QDBusObjectPath{ std::move(devPath).path() + "/" + QString::number(envNdx) };
	}

	DBusEnvironment::DBusEnvironment(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: QDBusAbstractInterface(std::move(service), std::move(path).path(), InterfaceName, std::move(connection), parent) {
	}
	DBusEnvironment::DBusEnvironment(QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: DBusEnvironment(QString(NUT_DBUS_URL), std::move(path), std::move(connection), parent) {
	}
	DBusEnvironment::DBusEnvironment(QDBusObjectPath path, QDBusAbstractInterface* from, QObject* parent)
	: DBusEnvironment(from->service(), std::move(path), from->connection(), parent) {
	}

	#define CLASS DBusEnvironment
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
	#undef CLASS



	const char* DBusInterface_IPv4::InterfaceName = NUT_DBUS_URL ".Interface_IPv4";

	QDBusObjectPath DBusInterface_IPv4::makePath(QString devName, qint32 envNdx, qint32 ifNdx) {
		return makePath(DBusEnvironment::makePath(std::move(devName), envNdx), ifNdx);
	}
	QDBusObjectPath DBusInterface_IPv4::makePath(QDBusObjectPath devPath, qint32 envNdx, qint32 ifNdx) {
		return makePath(DBusEnvironment::makePath(std::move(devPath), envNdx), ifNdx);
	}
	QDBusObjectPath DBusInterface_IPv4::makePath(QDBusObjectPath envPath, qint32 ifNdx) {
		return QDBusObjectPath{ std::move(envPath).path() + "/" + QString::number(ifNdx) };
	}

	DBusInterface_IPv4::DBusInterface_IPv4(QString service, QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: QDBusAbstractInterface(std::move(service), std::move(path).path(), InterfaceName, std::move(connection), parent) {
	}
	DBusInterface_IPv4::DBusInterface_IPv4(QDBusObjectPath path, QDBusConnection connection, QObject* parent)
	: DBusInterface_IPv4(QString(NUT_DBUS_URL), std::move(path), std::move(connection), parent) {
	}
	DBusInterface_IPv4::DBusInterface_IPv4(QDBusObjectPath path, QDBusAbstractInterface* from, QObject* parent)
	: DBusInterface_IPv4(from->service(), std::move(path), from->connection(), parent) {
	}

	#define CLASS DBusInterface_IPv4
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
	#undef CLASS

}
