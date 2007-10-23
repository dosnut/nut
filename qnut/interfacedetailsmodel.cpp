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
			case 0:  return tr("Assignment");
			case 1:  return tr("Netmask");
			case 2:  return tr("Gateway");
			default: break;
			}
			break;
		case IFDET_MOD_VALUE:
			switch (index.row()) {
			case 0:
				switch (interface->config.flags) {
				case CInterface::IF_STATIC:   return tr("static");
				case CInterface::IF_DYNAMIC:  return tr("dynamic (DHCP)");
				case CInterface::IF_ZEROCONF: return tr("dynamic (zeroconf)");
				case CInterface::IF_FALLBACK: return tr("dynamic with fallback");
				default: break;
				}
			case 1:
				if (interface->active)
					return interface->netmask.toString();
				else {
					switch (interface->config.flags) {
					case CInterface::IF_STATIC:
						return interface->config.staticNetmask.toString();
					case CInterface::IF_FALLBACK:
						return tr("none (fallback: %1)").arg(interface->config.staticNetmask.toString());
					default:
						return tr("none");
					}
				}
			case 2:
				if (interface->active)
					return interface->netmask.toString();
				else {
					switch (interface->config.flags) {
					case CInterface::IF_STATIC:
						return interface->config.staticGateway.toString();
					case CInterface::IF_FALLBACK:
						return tr("none (fallback: %1)").arg(interface->config.staticGateway.toString());
					default:
						return tr("none");
					}
				}
			default:
				break;
			}
			break;
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
