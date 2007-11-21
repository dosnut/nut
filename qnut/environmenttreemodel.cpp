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
#include "common.h"

#define ENVTREE_MOD_ITEM    0
#define ENVTREE_MOD_STATUS  1
#define ENVTREE_MOD_IP      2

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;
	
	CEnvironmentTreeModel::CEnvironmentTreeModel(CDevice * data, QObject * parent) : QAbstractItemModel(parent) {
		device = data;
		if (data) {
			foreach(CEnvironment * environment, device->environments) {
				//connect(environment, SIGNAL(activeChanged(bool)), this, SIGNAL(layoutChanged()));
				foreach(CInterface * interface, environment->interfaces) {
					connect(interface, SIGNAL(stateChanged(libnutcommon::InterfaceState)), this, SIGNAL(layoutChanged()));
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
		if ((device == NULL) || (!index.isValid()))
			return QVariant();
		
		QObject * currentData = static_cast<QObject *>(index.internalPointer());
		
		if ((role == Qt::DecorationRole) && (index.column() == ENVTREE_MOD_ITEM)) {
			if (currentData->parent() == device)
				return QIcon(UI_ICON_ENVIRONMENT);
			else
				switch (static_cast<CInterface *>(currentData)->state) {
				case IFS_OFF:
					return QIcon(UI_ICON_INTERFACE);
				case IFS_WAITFORCONFIG:
					return QIcon(UI_ICON_WARNING);
				default:
					return QIcon(UI_ICON_INTERFACE_ACTIVE);
				}
		}
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case ENVTREE_MOD_ITEM:
			if (currentData->parent() == device)
				return static_cast<CEnvironment *>(currentData)->name;
			else
				return '#' + QString::number(static_cast<CInterface *>(currentData)->index);
		case ENVTREE_MOD_STATUS:
			if (currentData->parent() == device) {
				return (static_cast<CEnvironment *>(currentData)->active) ? tr("active") : QString('-');
			}
			else {
				switch (static_cast<CInterface *>(currentData)->state) {
				case IFS_OFF:
					return tr("off");
				case IFS_STATIC:
					return tr("static");
				case IFS_DHCP:
					return tr("dynamic");
				case IFS_ZEROCONF:
					return tr("zeroconf");
				case IFS_WAITFORCONFIG:
					return tr("unconfigured");
				default:
					break;
				}
			}
			break;
		case ENVTREE_MOD_IP:
			if (currentData->parent() == device) {
				return QString('-');
			}
			else {
				CInterface * interface = static_cast<CInterface *>(currentData);
				if ((interface->state == IFS_OFF) || (interface->state == IFS_WAITFORCONFIG)) {
					if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC)
						return toStringDefault(interface->getConfig().getStaticIP());
					else if (interface->getConfig().getFlags() & IPv4Config::DO_USERSTATIC)
						return toStringDefault(interface->getUserConfig().ip());
					else
						return tr("none");
				}
				else
					return toStringDefault(interface->ip);
			}
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
			return createIndex(static_cast<CEnvironment *>(parentData)->index, 0, (void *)(parentData));
		else
			return QModelIndex();
	}
};
