//
// C++ Implementation: deviceoptionsmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QIcon>
#include "interfacedetailsmodel.h"
#include "constants.h"

#define IFDET_MOD_ITEM    0
#define IFDET_MOD_VALUE  1

namespace qnut {
	using namespace nut;
	//TODO: liste der dns server

	CInterfaceDetailsModel::CInterfaceDetailsModel(CInterface * data, QObject * parent) : QAbstractItemModel(parent) {
		interface = data;
	}
	
	CInterfaceDetailsModel::~CInterfaceDetailsModel() {
		interface = NULL;
	}
	
	void CInterfaceDetailsModel::setInterface(CInterface * data) {
		interface = data;
	}
	
	int CInterfaceDetailsModel::columnCount(const QModelIndex &) const {
		if (interface == NULL)
			return 0;
		else
			return 2;
	}
	
	int CInterfaceDetailsModel::rowCount(const QModelIndex & parent) const {
		if (interface == NULL)
			return 0;
		
		if (!parent.isValid())
			return 3;
		else {
			return 0;
		}
	}
	
	QVariant CInterfaceDetailsModel::data(const QModelIndex & index, int role) const {
		if (interface == NULL)
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case IFDET_MOD_ITEM:
			switch (index.row()) {
			case 0:
				return tr("Assignment");
			case 1:
				return tr("Netmask");
			case 2:
				return tr("Gateway");
			default:
				break;
			}
			break;
		case IFDET_MOD_VALUE: {
			QString result = tr("unknown");
			switch (index.row()) {
			case 0:
				switch (interface->state) {
				case IFS_OFF:
					if (interface->getConfig().getFlags() & IPv4Config::DO_DHCP) {
						result = tr("dynamic (DHCP)");
						if (interface->getConfig().getFlags() & IPv4Config::DO_ZEROCONF) {
							return result + ' ' + tr("fallback: zeroconf");
						}
						else if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC) {
							return result + ' ' + tr("fallback: static");
						}
					}
					else if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC) {
						result = tr("static");
						if (interface->getConfig().getFlags() & IPv4Config::MAY_USERSTATIC) {
							return result + " (" + tr("customizable") + ')';
						}
					}
					return result;
				case IFS_STATIC:
					if (interface->getConfig().getFlags() & IPv4Config::DO_DHCP)
						return tr("static (fallback)");
					else
						return tr("static");
				case IFS_DHCP:
					return tr("dynamic");
				case IFS_ZEROCONF:
					if (interface->getConfig().getFlags() & IPv4Config::DO_DHCP)
						return tr("zeroconf (fallback)");
					else
						return tr("zeroconf");
				default:
					break;
				}
			case 1:
				if (interface->state == IFS_OFF) {
					if (interface->getConfig().getFlags() & IPv4Config::DO_DHCP) {
						return tr("none");
					}
					else if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC) {
						return interface->getConfig().getStaticNetmask().toString();
					}
					else
						return tr("unknown");
				}
				else
					return interface->netmask.toString();
			case 2:
				if (interface->state == IFS_OFF) {
					if (interface->getConfig().getFlags() & IPv4Config::DO_DHCP) {
						return tr("none");
					}
					else if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC) {
						return interface->getConfig().getStaticGateway().toString();
					}
					else
						return tr("unknown");
				}
				else
					return interface->gateway.toString();
			default:
				break;
			}
			break;
			}
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CInterfaceDetailsModel::flags(const QModelIndex & index) const {
		if (interface == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CInterfaceDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (interface == NULL)
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
		if (interface == NULL)
			return QModelIndex();
		
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		
		
		if (!parent.isValid()) {
			if (row < 4)
				return createIndex(row, column, (void *)NULL);
		}
		
		return QModelIndex();
	}
	
	QModelIndex CInterfaceDetailsModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
};
