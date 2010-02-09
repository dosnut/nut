//
// C++ Implementation: environmenttreemodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QIcon>
#include <libnutclient/cdevice.h>
#include <libnutclient/cenvironment.h>
#include <libnutclient/cinterface.h>
#include "environmenttreemodel.h"
#include "common.h"
#include "constants.h"

#define ENVTREE_MOD_ITEM    0
#define ENVTREE_MOD_STATUS  1
#define ENVTREE_MOD_IP      2

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;
	
	CEnvironmentTreeModel::CEnvironmentTreeModel(CDevice * data, QObject * parent) : QAbstractItemModel(parent) {
		m_Device = data;
		if (data) {
			foreach(CEnvironment * environment, m_Device->getEnvironments()) {
				connect(environment, SIGNAL(activeChanged(bool)), this, SIGNAL(layoutChanged()));
				foreach(CInterface * interface, environment->getInterfaces()) {
					connect(interface, SIGNAL(stateChanged(libnutcommon::InterfaceState)), this, SIGNAL(layoutChanged()));
				}
			}
		}
	}
	
	CEnvironmentTreeModel::~CEnvironmentTreeModel() {
		m_Device = NULL;
	}
	
	int CEnvironmentTreeModel::columnCount(const QModelIndex &) const {
		if (m_Device == NULL)
			return 0;
		else
			return 3;
	}
	
	int CEnvironmentTreeModel::rowCount(const QModelIndex & parent) const {
		if (m_Device == NULL)
			return 0;
		
		if (!parent.isValid())
			return m_Device->getEnvironments().count();
		else {
			QObject * parentData = static_cast<QObject *>(parent.internalPointer());
			if (parentData->parent() == m_Device)
				return static_cast<CEnvironment *>(parentData)->getInterfaces().count();
			else
				return 0;
		}
	}
	
	QVariant CEnvironmentTreeModel::data(const QModelIndex & index, int role) const {
		if ((m_Device == NULL) || (!index.isValid()))
			return QVariant();
		
		QObject * currentData = static_cast<QObject *>(index.internalPointer());
		
		if ((role == Qt::DecorationRole) && (index.column() == ENVTREE_MOD_ITEM)) {
			if (currentData->parent() == m_Device)
				return QIcon(static_cast<CEnvironment *>(currentData) == m_Device->getActiveEnvironment() ? UI_ICON_ENVIRONMENT_ACTIVE : UI_ICON_ENVIRONMENT);
			else
				switch (static_cast<CInterface *>(currentData)->getState()) {
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
			if (currentData->parent() == m_Device) {
				return getNameDefault(static_cast<CEnvironment *>(currentData));
			}
			else
				return '#' + QString::number(static_cast<CInterface *>(currentData)->getIndex());
		case ENVTREE_MOD_STATUS:
			if (currentData->parent() == m_Device) {
				return (static_cast<CEnvironment *>(currentData)->getState()) ? tr("active") : QString('-');
			}
			else {
				return toStringTr(static_cast<CInterface *>(currentData)->getState());
			}
			break;
		case ENVTREE_MOD_IP:
			if (currentData->parent() == m_Device) {
				return QString('-');
			}
			else {
				CInterface * interface = static_cast<CInterface *>(currentData);
				if ((interface->getState() == IFS_OFF) || (interface->getState() == IFS_WAITFORCONFIG)) {
					if (interface->getConfig().getFlags() & IPv4Config::DO_STATIC)
						return toStringDefault(interface->getConfig().getStaticIP());
					else if (interface->getConfig().getFlags() & IPv4Config::DO_USERSTATIC)
						return toStringDefault(interface->getUserConfig().ip());
					else
						return tr("none");
				}
				else
					return toStringDefault(interface->getIp());
			}
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CEnvironmentTreeModel::flags(const QModelIndex & index) const {
		if (m_Device == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CEnvironmentTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (m_Device == NULL)
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
		if (m_Device == NULL)
			return QModelIndex();
		
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		
		if (!parent.isValid()) {
			return createIndex(row, column, m_Device->getEnvironments()[row]);
		}
		else {
			CEnvironment * parentData = static_cast<CEnvironment *>(parent.internalPointer());
			return createIndex(row, column, parentData->getInterfaces()[row]);
		}
	}
	
	QModelIndex CEnvironmentTreeModel::parent(const QModelIndex & index) const {
		if (m_Device == NULL)
			return QModelIndex();
		
		if (!index.isValid())
			return QModelIndex();
		
		if (index.internalPointer() == NULL)
			return QModelIndex();
		
		QObject * parentData = static_cast<QObject *>(index.internalPointer())->parent();
		
		if (parentData->parent() == m_Device)
			return createIndex(static_cast<CEnvironment *>(parentData)->getIndex(), 0, (void *)(parentData));
		else
			return QModelIndex();
	}
}
