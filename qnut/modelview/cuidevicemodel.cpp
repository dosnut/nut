//
// C++ Implementation: cuidevicemodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QIcon>

#include <libnutclient/client.h>

#include "modelview/cuidevicemodel.h"
#include "cdevicesettings.h"
#include "cuidevice.h"
#include "common.h"

#define OV_MOD_NAME     0
#define OV_MOD_STATUS   1
#define OV_MOD_TYPE     2
#define OV_MOD_ENV      3
#define OV_MOD_IP       4
#define OV_MOD_NETWORK  5
#define OV_MOD_COLCOUNT 6

namespace qnut {
	using namespace libnutclient;
	using namespace libnutwireless;
	using namespace libnutcommon;

	CUIDeviceModel::CUIDeviceModel(QWidget* parent)
	: QAbstractItemModel(parent)
	, m_DeviceSettings(new CDeviceSettings(parent)) {
	}

	CUIDeviceModel::~CUIDeviceModel() {
		qDeleteAll(m_UIDevices);
		m_UIDevices.clear();
	}

	CUIDevice* CUIDeviceModel::addUIDevice(CDevice* device) {
		CUIDevice* newDevice = new CUIDevice(device, m_DeviceSettings.get());
		beginInsertRows(QModelIndex(), m_UIDevices.size(), m_UIDevices.size());
		m_UIDevices.append(newDevice);

		connect(device, &CDevice::stateChanged,
			this, &CUIDeviceModel::updateDeviceState);
#ifndef NUT_NO_WIRELESS
		if (device->getWireless()) {
			connect(newDevice, &CUIDevice::wirelessInformationUpdated,
				this, &CUIDeviceModel::updateSignalQuality);
		}
#endif
		endInsertRows();
		return newDevice;
	}

	void CUIDeviceModel::removeUIDevice(CUIDevice* target) {
		int targetPos = m_UIDevices.indexOf(target);
		removeUIDevice(targetPos);
	}

	void CUIDeviceModel::removeUIDevice(int position) {
		if (position < 0 || position >= m_UIDevices.size()) return;

		beginRemoveRows(QModelIndex(), position, position);
		CUIDevice * target = m_UIDevices.takeAt(position);
		disconnect(target->device(), &CDevice::stateChanged,
			this, &CUIDeviceModel::updateDeviceState);
#ifndef NUT_NO_WIRELESS
		if (target->device()->getWireless()) {
			connect(target, &CUIDevice::wirelessInformationUpdated,
				this, &CUIDeviceModel::updateSignalQuality);
		}
#endif
		delete target;
		endRemoveRows();
	}

	int CUIDeviceModel::findUIDevice(CDevice* device) {
		for (int targetPos = 0; targetPos < m_UIDevices.size(); ++targetPos) {
			if (m_UIDevices[targetPos]->device() == device) {
				return targetPos;
			}
		}
		return -1;
	}

	void CUIDeviceModel::updateDeviceState() {
		int targetPos = findUIDevice(qobject_cast<CDevice*>(sender()));
		if (targetPos == -1) return;

		emit dataChanged(index(targetPos, 0), index(targetPos, OV_MOD_COLCOUNT-1));
	}

	void CUIDeviceModel::updateSignalQuality() {
		int targetPos = m_UIDevices.indexOf(qobject_cast<CUIDevice*>(sender()));
		if (targetPos == -1) return;

		emit dataChanged(index(targetPos, OV_MOD_NETWORK), index(targetPos, OV_MOD_NETWORK));
	}

	Qt::ItemFlags CUIDeviceModel::flags(const QModelIndex & index) const {
		if (!index.isValid()) return 0;

		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QModelIndex CUIDeviceModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}

	int CUIDeviceModel::columnCount(const QModelIndex & /*parent*/) const {
		return OV_MOD_COLCOUNT;
	}

	QModelIndex CUIDeviceModel::index(int row, int column, const QModelIndex & /*parent*/) const {
		return (row >= m_UIDevices.size())
			? QModelIndex()
			: createIndex(row, column, reinterpret_cast<void*>(m_UIDevices.at(row)));
	}

	bool CUIDeviceModel::hasChildren(const QModelIndex & parent) const {
		return !parent.isValid();
	}

	int CUIDeviceModel::rowCount(const QModelIndex & parent) const {
		return parent.isValid() ? 0 : m_UIDevices.count();
	}

	QVariant CUIDeviceModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid()) return QVariant();

		CDevice* data = reinterpret_cast<CUIDevice*>(index.internalPointer())->device();

		if (role == Qt::DisplayRole) {
			switch (index.column()) {
			case OV_MOD_NAME:
				return data->getName();
			case OV_MOD_STATUS:
				return toStringTr(data->getState());
			case OV_MOD_TYPE:
				return toStringTr(data->getType());
			case OV_MOD_IP:
				if (data->getState() != DeviceState::UP) {
					return QString('-');
				}
				else {
					return activeIP(data);
				}
			case OV_MOD_ENV:
				if (data->getState() >= DeviceState::UNCONFIGURED) {
					return getNameDefault(data->getActiveEnvironment());
				}
				else {
					return tr("none");
				}
			case OV_MOD_NETWORK:
				if (data->getState() > DeviceState::ACTIVATED) {
					return currentNetwork(data);
				}
				else {
					return QString('-');
				}
			default:
				break;
			}
		}
		else if (role == Qt::DecorationRole) {
			if (index.column() == 0) {
				return QIcon(iconFile(data));
			}
		}
		return QVariant();
	}

	QVariant CUIDeviceModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (role != Qt::DisplayRole) return QVariant();

		if (orientation == Qt::Horizontal) {
			switch (section) {
			case OV_MOD_NAME:
				return tr("Device");
			case OV_MOD_STATUS:
				return tr("Status");
			case OV_MOD_TYPE:
				return tr("Type");
			case OV_MOD_IP:
				return tr("IP-Address");
			case OV_MOD_ENV:
				return tr("Environment");
			case OV_MOD_NETWORK:
				return tr("Network");
			default:
				break;
			}
		}
		return QVariant();
	}
}
