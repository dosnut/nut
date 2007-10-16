//
// C++ Implementation: overviewlistmodel
//
// Description: 
// Dies stellt die Implementierung für das Model des ListView Steuerelemnts dar.
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QIcon>
#include "overviewmodel.h"
#include "common.h"

#define OV_MOD_NAME    0
#define OV_MOD_STATUS  1
#define OV_MOD_TYPE    2
#define OV_MOD_ENV     3
#define OV_MOD_IP      4

namespace qnut {
	COverViewModel::COverViewModel(CDeviceList * deviceList, QObject * parent) : QAbstractItemModel(parent) {
		devices = deviceList;
	}
	
	COverViewModel::~COverViewModel() {
		devices = NULL;
	}
	
	Qt::ItemFlags COverViewModel::flags(const QModelIndex & index) const {
		if (devices == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QModelIndex COverViewModel::parent(const QModelIndex & index) const {
		return QModelIndex();
	}
	
	int COverViewModel::columnCount(const QModelIndex & parent) const {
		if (devices == NULL)
			return 0;
			
		if (parent.isValid())
			return 0;
		
		return 5;
	}
	
	QModelIndex COverViewModel::index(int row, int column, const QModelIndex & parent) const {
		if ((devices != NULL) && (row < devices->count()) && (!parent.isValid()))
			return createIndex(row, column, (void *)(devices->at(row)));
		else
			return QModelIndex();
	}
	
	int COverViewModel::rowCount(const QModelIndex & parent) const {
		if (devices == NULL)
			return 0;
		
		if (!parent.isValid())
			return devices->count();
		
		return 0;
	}
	
	QVariant COverViewModel::data(const QModelIndex & index, int role) const {
		if (devices == NULL)
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
		if (index.row() >= devices->size())
			return QVariant();
		
		CDevice * data = (CDevice *)(index.internalPointer());
		
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
			case OV_MOD_NAME:
				return data->name;
			case OV_MOD_STATUS:
				return toString(data->state);
			case OV_MOD_TYPE:
				return toString(data->type);
			case OV_MOD_IP: {
					if (data->state != DS_UP)
						return QString('-');
					else
						return activeIP(data);
				}
			case OV_MOD_ENV: {
					if (data->state != DS_UP)
						return tr("none");
					else
						return data->activeEnvironment->name;
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
	
	QVariant COverViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (devices == NULL)
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case OV_MOD_NAME:
				return tr("Name");
			case OV_MOD_STATUS:
				return tr("Status");
			case OV_MOD_TYPE:
				return tr("Type");
			case OV_MOD_IP:
				return tr("assigned IP-Address");
			case OV_MOD_ENV:
				return tr("Environment");
			default:
				break;
			}
		}
		return QVariant();
	}
};
