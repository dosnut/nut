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

#define OVMOD_NAME    0
#define OVMOD_STATUS  1
#define OVMOD_TYPE    2
#define OVMOD_IP      3

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
        
        return 4;
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
            case OVMOD_NAME:
                return data->name;
            case OVMOD_STATUS:
                return toString(data->state);
            case OVMOD_TYPE:
                return toString(data->type);
            case OVMOD_IP: {
                    if (data->state != DS_UP)
                        return QString('-');
                    
                    return activeIP(data->activeEnvironment);
                }
            default:
                break;
            }
        }
        else if (role == Qt::DecorationRole) {
            if (index.column() == 0) {
                return QIcon(getDeviceIcon(data));
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
            case OVMOD_NAME:
                return tr("Name");
            case OVMOD_STATUS:
                return tr("Status");
            case OVMOD_TYPE:
                return tr("Type");
            case OVMOD_IP:
                return tr("assigned IP-Addess");
            default:
                break;
            }
        }
        return QVariant();
    }
};
