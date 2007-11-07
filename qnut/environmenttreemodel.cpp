//
// C++ Implementation: environmenttreemodel
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
#include "environmenttreemodel.h"
#include "constants.h"

#define ENVTREE_MOD_ITEM    0
#define ENVTREE_MOD_STATUS  1
#define ENVTREE_MOD_IP      2

namespace qnut {
	using namespace nut;
	CEnvironmentTreeModel::CEnvironmentTreeModel(CDevice * data, QObject * parent) : QAbstractItemModel(parent) {
		device = data;
		if (data) {
			foreach(CEnvironment * environment, device->environments) {
				//connect(environment, SIGNAL(activeChanged(bool)), this, SIGNAL(layoutChanged()));
				foreach(CInterface * interface, environment->interfaces) {
					connect(interface, SIGNAL(stateChanged(InterfaceState)), this, SIGNAL(layoutChanged()));
				}
			}
		}
	}
	
	CEnvironmentTreeModel::~CEnvironmentTreeModel() {
		device = NULL;
	}
	
	int CEnvironmentTreeModel::columnCount(const QModelIndex &) const {
		if (device == NULL)
			return 0;
		else
			return 3;
	}
	
	int CEnvironmentTreeModel::rowCount(const QModelIndex & parent) const {
		if (device == NULL)
			return 0;
		
		if (!parent.isValid())
			return device->environments.count();
		else {
			QObject * parentData = static_cast<QObject *>(parent.internalPointer());
			if (parentData->parent() == device)
				return static_cast<CEnvironment *>(parentData)->interfaces.count();
			else
				return 0;
		}
	}
	
	QVariant CEnvironmentTreeModel::data(const QModelIndex & index, int role) const {
		if (device == NULL)
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
		QObject * data = static_cast<QObject *>(index.internalPointer());
		switch (index.column()) {
		case ENVTREE_MOD_ITEM:
			if (data->parent() == device) {
				switch (role) {
				case Qt::DisplayRole:
					return static_cast<CEnvironment *>(data)->name;
				case Qt::DecorationRole:
					return QIcon(UI_ICON_ENVIRONMENT);
				default:
					break;
				}
			}
			else {
				CInterface * interface = static_cast<CInterface *>(data);
				CEnvironment * environment = static_cast<CEnvironment *>(interface->parent());
				switch (role) {
				case Qt::DisplayRole:
					return tr("#%1").arg(environment->interfaces.indexOf(interface));
				case Qt::DecorationRole:
					return QIcon((interface->state == IFS_OFF) ? UI_ICON_INTERFACE_INACTIVE : UI_ICON_INTERFACE_ACTIVE);
				default:
					break;
				}
			}
			break;
		case ENVTREE_MOD_STATUS:
			if (role == Qt::DisplayRole)
				break;
			
			if (data->parent() == device) {
				return (static_cast<CEnvironment *>(data) == device->activeEnvironment) ? tr("active") : QString('-');
			}
			else {
				switch (static_cast<CInterface *>(data)->state) {
				case IFS_OFF:
					return tr("off");
				case IFS_STATIC:
					return tr("static");
				case IFS_DHCP:
					return tr("dynamic");
				case IFS_ZEROCONF:
					return tr("zeroconf");
				default:
					break;
				}
			}
			break;
		case ENVTREE_MOD_IP:
			if (role == Qt::DisplayRole)
				break;
			
			if (data->parent() == device) {
				return QString('-');
			}
			else {
				CInterface * interface = static_cast<CInterface *>(data);
				if (interface->state == IFS_OFF) {
					if (interface->getConfig().getFlags() & IPv4Config::DO_DHCP)
						return tr("none");
					else if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC)
						return interface->getConfig().getStaticIP().toString();
					else if (interface->getConfig().getFlags() & IPv4Config::DO_USERSTATIC)
						return interface->getUserConfig().ip().toString();
					else
						return tr("unknown");
				}
				else
					return interface->ip.toString();
			}
			break;
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CEnvironmentTreeModel::flags(const QModelIndex & index) const {
		if (device == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CEnvironmentTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (device == NULL)
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case ENVTREE_MOD_ITEM:
				return tr("Item");
			case ENVTREE_MOD_STATUS:
				return tr("Status");
			case ENVTREE_MOD_IP:
				return tr("IP-Address");
			default:
				break;
			}
		}
		return QVariant();
	}
	
	QModelIndex CEnvironmentTreeModel::index(int row, int column, const QModelIndex & parent) const {
		if (device == NULL)
			return QModelIndex();
		
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		
		if (!parent.isValid()) {
			return createIndex(row, column, device->environments[row]);
		}
		else {
			CEnvironment * parentData = static_cast<CEnvironment *>(parent.internalPointer());
			return createIndex(row, column, parentData->interfaces[row]);
		}
		
		return QModelIndex();
	}
	
	QModelIndex CEnvironmentTreeModel::parent(const QModelIndex & index) const {
		if (device == NULL)
			return QModelIndex();
		
		if (!index.isValid())
			return QModelIndex();
		
		if (index.internalPointer() == NULL)
			return QModelIndex();
		
		QObject * parentData = static_cast<QObject *>(index.internalPointer())->parent();
		
		if (parentData->parent() == device)
			return createIndex(device->environments.indexOf(static_cast<CEnvironment *>(parentData)), 0, (void *)(parentData));
		else
			return QModelIndex();
	}
};
