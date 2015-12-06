#include "cdevicemanager.h"
#include "libnutcommon/common.h"
#include "libnutclientbase/dbus.h"
#include "cdevice.h"

namespace libnutclient {
	using namespace libnutcommon;

	CDeviceManager::CDeviceManager(CNutService* parent)
	: CNutServiceClient(parent) {
		m_dbusDevices.reserve(10);
	}

	CDeviceManager::~CDeviceManager() {
		dbusLostService();
	}

	void CDeviceManager::dbusLostService() {
		if (m_dbusDevmgr) {
			m_dbusDevmgr->deleteLater();
			m_dbusDevmgr = nullptr;
		}

		clear();
	}

	void CDeviceManager::dbusConnectService(QString service, QDBusConnection connection) {
		if (m_dbusDevmgr) return;

		m_dbusDevmgr = new libnutclientbase::DBusDeviceManager(service, libnutclientbase::DBusDeviceManager::makePath(), connection, this);
		connect(m_dbusDevmgr, &libnutclientbase::DBusDeviceManager::deviceAddedPath, this, &CDeviceManager::dbusDeviceAdded);
		connect(m_dbusDevmgr, &libnutclientbase::DBusDeviceManager::deviceRemovedPath, this, &CDeviceManager::dbusDeviceRemoved);

		reload();
	}

	void CDeviceManager::clear() {
		/* mark all created devices for deletion */
		for (auto const& dev: m_dbusDevices) {
			// QHash iterator only iterates values it seems...
			dev->deleteLater();
		}
		m_dbusDevices.clear();

		/* clear "visible" devices list, but keep a copy for notifications */
		decltype(m_devices) old_devices;
		old_devices.swap(m_devices);

		/* remove notifications for "visible" devices */
		for (auto const& dev: old_devices) {
			emit deviceRemoved(dev);
		}
	}

	void CDeviceManager::reload() {
		if (!m_dbusDevmgr) return;

		m_dbusDevmgr->getDeviceList([this](QList<QDBusObjectPath> devPaths) {
			for (auto const& devPath: devPaths) {
				load(devPath);
			}
		}, [this](QDBusError error) {
			emit dbusError("DeviceManager.getDeviceList", error);
		}, this);
	}

	void CDeviceManager::load(const QDBusObjectPath& devPath) {
		if (m_dbusDevmgr && !m_dbusDevices.contains(devPath)) {
			auto device = new CDevice(this, devPath);
			m_dbusDevices.insert(devPath, device);
		}
	}

	void CDeviceManager::rebuildIndex() {
		int i = 0;
		for (auto const& dev: m_devices) {
			dev->m_index = i++;
		}
	}

	void CDeviceManager::deviceInitializationFailed(CDevice* device) {
		if (!device) return;
		auto devPath = device->path();

		if (device != m_dbusDevices.value(devPath, nullptr)) {
			device->deleteLater();
			return;
		}

		if (m_devices.contains(device)) return; /* already completed */

		m_dbusDevices.remove(devPath);
		device->deleteLater();

		contextLog(tr("Couldn't load device %1").arg(devPath.path()));
	}

	void CDeviceManager::deviceInitializationCompleted(CDevice* device) {
		if (!device) return;
		auto devPath = device->path();

		if (device != m_dbusDevices.value(devPath, nullptr)) {
			device->deleteLater();
			return;
		}

		if (m_devices.contains(device)) return; /* already completed */

		device->m_index = m_devices.size(); // Set device index
		m_devices.append(device);
		emit deviceAdded(device);
	}

	void CDeviceManager::dbusDeviceAdded(const QDBusObjectPath &devPath) {
		load(devPath);
	}

	void CDeviceManager::dbusDeviceRemoved(const QDBusObjectPath &devPath) {
		auto device = m_dbusDevices.value(devPath, nullptr);
		if (!device) return;

		m_dbusDevices.remove(devPath);

		if (m_devices.removeAll(device) > 0) {
			rebuildIndex();
			emit deviceRemoved(device);
		}
		device->deleteLater();
	}

	bool CDeviceManager::createBridge(QList<CDevice *> /* devices */) {
		return false;
	}

	bool CDeviceManager::destroyBridge(CDevice * /* device */) {
		return false;
	}

	void CDeviceManager::refreshAll() {
		clear();
		reload();
	}
}
