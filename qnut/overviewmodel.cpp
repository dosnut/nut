//
// C++ Implementation: overviewlistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QIcon>
#include "overviewmodel.h"
#include "common.h"

#define OV_MOD_NAME    0
#define OV_MOD_STATUS  1
#define OV_MOD_TYPE    2
#define OV_MOD_ENV     3
#define OV_MOD_IP      4
#define OV_MOD_NETWORK 5

namespace qnut {
	using namespace libnutclient;
	using namespace libnutwireless;
	using namespace libnutcommon;
	
	COverViewModel::COverViewModel(CDeviceManager * deviceManager, QObject * parent) : QAbstractItemModel(parent) {
		if (deviceManager) {
			m_Devices = &(deviceManager->getDevices());
			
			connect(deviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(deviceAdded(libnutclient::CDevice *)));
			connect(deviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(deviceRemoved(libnutclient::CDevice *)));
		}
		else
			m_Devices = NULL;
	}
	
	COverViewModel::~COverViewModel() {
		m_Devices = NULL;
	}
	
	void COverViewModel::deviceAdded(CDevice * device) {
		connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SIGNAL(layoutChanged()));
		#ifndef QNUT_NO_WIRELESS
		if (device->getWpaSupplicant())
			connect(device->getWpaSupplicant(), SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)), this, SIGNAL(layoutChanged()));
		#endif
		emit layoutChanged();
	}
	
	void COverViewModel::deviceRemoved(CDevice * device) {
		disconnect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SIGNAL(layoutChanged()));
		#ifndef QNUT_NO_WIRELESS
		if (device->getWpaSupplicant())
			disconnect(device->getWpaSupplicant(), SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)), this, SIGNAL(layoutChanged()));
		#endif
		emit layoutChanged();
	}
	
	Qt::ItemFlags COverViewModel::flags(const QModelIndex & index) const {
		if (m_Devices == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QModelIndex COverViewModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
	
	int COverViewModel::columnCount(const QModelIndex & parent) const {
		if (m_Devices == NULL)
			return 0;
			
		if (parent.isValid())
			return 0;
		
		return 6;
	}
	
	QModelIndex COverViewModel::index(int row, int column, const QModelIndex & parent) const {
		if ((m_Devices == NULL) || (parent.isValid()) || (row >= m_Devices->size()))
			return QModelIndex();
		else
			return createIndex(row, column, (void *)(m_Devices->at(row)));
	}
	
	int COverViewModel::rowCount(const QModelIndex & parent) const {
		if (m_Devices == NULL)
			return 0;
		
		if (!parent.isValid())
			return m_Devices->count();
		
		return 0;
	}
	
	QVariant COverViewModel::data(const QModelIndex & index, int role) const {
		if (m_Devices == NULL)
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
/*		if (index.row() >= m_Devices->size())
			return QVariant();*/
		
		CDevice * data = (CDevice *)(index.internalPointer());
		
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
			case OV_MOD_NAME:
				return data->getName();
			case OV_MOD_STATUS:
				return toStringTr(data->getState());
			case OV_MOD_TYPE:
				return toStringTr(data->getType());
			case OV_MOD_IP: {
					if (data->getState() != DS_UP)
						return QString('-');
					else
						return activeIP(data);
				}
			case OV_MOD_ENV:
				if (data->getState() >= DS_UNCONFIGURED)
					return data->getActiveEnvironment()->getName();
				else
					return tr("none");
			case OV_MOD_NETWORK:
				if (data->getState() > DS_ACTIVATED) {
					#ifndef QNUT_NO_WIRELESS
					if (data->getWpaSupplicant()) {
						WextSignal signal = data->getWpaSupplicant()->getSignalQuality();
						return data->getEssid() + " (" +
							QString::number(signal.quality.value) + '/'+
							QString::number(signal.quality.maximum) + ')';
					}
					else
					#endif
						return (data->getType() == DT_AIR) ? data->getEssid() : tr("local");
				}
				else
					return QString('-');
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
	
	QVariant COverViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (m_Devices == NULL)
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
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
