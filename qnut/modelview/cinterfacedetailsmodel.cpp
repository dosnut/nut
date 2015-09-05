//
// C++ Implementation: deviceoptionsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <libnutclient/client.h>
#include <QIcon>

#include "modelview/cinterfacedetailsmodel.h"
#include "common.h"

#define IFDET_MOD_ITEM   0
#define IFDET_MOD_VALUE  1

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;

	CInterfaceDetailsModel::CInterfaceDetailsModel(CInterface * data, QObject * parent) : QAbstractItemModel(parent) {
		m_Interface = data;
		if (m_Interface) {
			connect(m_Interface, SIGNAL(stateChanged(libnutcommon::InterfaceState)), this, SIGNAL(layoutChanged()));
			//connect(m_Interface, SIGNAL(userConfigApplied()), this, SIGNAL(layoutChanged()));
		}
	}

	CInterfaceDetailsModel::~CInterfaceDetailsModel() {
		m_Interface = NULL;
	}

	int CInterfaceDetailsModel::columnCount(const QModelIndex &) const {
		if (m_Interface == NULL)
			return 0;
		else
			return 2;
	}

	int CInterfaceDetailsModel::rowCount(const QModelIndex & parent) const {
		if (m_Interface == NULL)
			return 0;

		if (!parent.isValid())
			if (m_Interface->getState() == InterfaceState::WAITFORCONFIG)
				return 4 + m_Interface->getUserConfig().dnsServers.size();
			else if (m_Interface->getState() != InterfaceState::OFF)
				return 4 + m_Interface->getDnsServers().size();
			else if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
				return 4 + m_Interface->getConfig().static_dnsservers.size();
			else
				return 4;
		else {
			return 0;
		}
	}

	QVariant CInterfaceDetailsModel::data(const QModelIndex & index, int role) const {
		if (m_Interface == NULL)
			return QVariant();

		if (!index.isValid())
			return QVariant();

		if (role != Qt::DisplayRole)
			return QVariant();

		switch (index.column()) {
		case IFDET_MOD_ITEM:
			switch (index.row()) {
			case 0:
				return tr("Type");
			case 1:
				return tr("IP-Address");
			case 2:
				return tr("Netmask");
			case 3:
				return tr("Gateway");
			default:
				return tr("DNS server #%1").arg(index.row()-4);
			}
			break;
		case IFDET_MOD_VALUE:
			switch (index.row()) {
			case 0:
				switch (m_Interface->getState()) {
				case InterfaceState::OFF:
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::USERSTATIC)
						return tr("user defined static");
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::DHCP) {
						QString fallback;
						if (m_Interface->getConfig().flags & IPv4ConfigFlag::ZEROCONF)
							fallback = ' ' + tr("fallback: zeroconf");
						else if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
							fallback = ' ' + tr("fallback: static");
						return tr("dynamic (DHCP)") + fallback;
					}
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::ZEROCONF)
						return tr("zeroconf");
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
						return tr("static");
					break;
				case InterfaceState::STATIC:
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::DHCP)
						return tr("static (fallback)");
					else
						return tr("static");
				case InterfaceState::DHCP:
					return tr("dynamic");
				case InterfaceState::ZEROCONF:
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::DHCP)
						return tr("zeroconf (fallback)");
					else
						return tr("zeroconf");
				default:
					break;
				}
				return tr("unknown");
			case 1:
				if ((m_Interface->getState() == InterfaceState::OFF) || (m_Interface->getState() == InterfaceState::WAITFORCONFIG)) {
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
						return toStringDefault(m_Interface->getConfig().static_ip);
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::USERSTATIC)
						return toStringDefault(m_Interface->getUserConfig().ip);
					else
						return tr("none");
				}
				else
					return toStringDefault(m_Interface->getIp());
			case 2:
				if ((m_Interface->getState() == InterfaceState::OFF) || (m_Interface->getState() == InterfaceState::WAITFORCONFIG)) {
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
						return toStringDefault(m_Interface->getConfig().static_netmask);
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::USERSTATIC)
						return toStringDefault(m_Interface->getUserConfig().netmask);
					else
						return tr("none");
				}
				else
					return toStringDefault(m_Interface->getNetmask());
			case 3:
				if ((m_Interface->getState() == InterfaceState::OFF) || (m_Interface->getState() == InterfaceState::WAITFORCONFIG)) {
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
						return toStringDefault(m_Interface->getConfig().static_gateway);
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::USERSTATIC)
						return toStringDefault(m_Interface->getUserConfig().gateway);
					else
						return tr("none");
				}
				else
					return toStringDefault(m_Interface->getGateway());
			default:
				if ((m_Interface->getState() == InterfaceState::OFF) || (m_Interface->getState() == InterfaceState::WAITFORCONFIG)) {
					if (m_Interface->getConfig().flags & IPv4ConfigFlag::STATIC)
						return m_Interface->getConfig().static_dnsservers[index.row()-4].toString();
					else if (m_Interface->getConfig().flags & IPv4ConfigFlag::USERSTATIC)
						return m_Interface->getUserConfig().dnsServers[index.row()-4].toString();
					else
						break;
				}
				else
					return m_Interface->getDnsServers()[index.row()-4].toString();
			}
			break;
		default:
			break;
		}

		return QVariant();
	}

	Qt::ItemFlags CInterfaceDetailsModel::flags(const QModelIndex & index) const {
		if (m_Interface == NULL)
			return 0;

		if (!index.isValid())
			return 0;

		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant CInterfaceDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (m_Interface == NULL)
			return QVariant();

		if (role != Qt::DisplayRole)
			return QVariant();

		if (orientation == Qt::Horizontal) {
			switch (section) {
			case IFDET_MOD_ITEM:
				return tr("Detail");
			case IFDET_MOD_VALUE:
				return tr("Value");
			default:
				break;
			}
		}
		return QVariant();
	}

	QModelIndex CInterfaceDetailsModel::index(int row, int column, const QModelIndex & parent) const {
		if (m_Interface == NULL)
			return QModelIndex();

		if (!hasIndex(row, column, parent))
			return QModelIndex();

		if (!parent.isValid()) {
			//if (row < 4 + m_Interface->dnsserver.size())
			return createIndex(row, column, nullptr);
		}

		return QModelIndex();
	}

	QModelIndex CInterfaceDetailsModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
}
