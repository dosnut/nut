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
#include "overviewlistmodel.h"
#include "common.h"

namespace qnut {
    COverViewListModel::COverViewListModel(CDeviceList * deviceList, QObject * parent) : QAbstractItemModel(parent) {
        devices = deviceList;
    }
    
    COverViewListModel::~COverViewListModel() {
        devices = NULL;
    }
    
    Qt::ItemFlags COverViewListModel::flags(const QModelIndex & index) const {
        if (devices == NULL)
            return 0;
        
        if (!index.isValid())
            return 0;
        
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    
    QModelIndex COverViewListModel::parent(const QModelIndex & index) const {
        return QModelIndex();
    }
    
    int COverViewListModel::columnCount(const QModelIndex & parent) const {
        if (devices == NULL)
            return 0;
            
        if (parent.isValid())
            return 0;
        
        return 3;
    }
    
    QModelIndex COverViewListModel::index(int row, int column, const QModelIndex & parent) const {
        if ((devices != NULL) && (row < devices->count()) && (!parent.isValid()))
            return createIndex(row, column, (void *)(devices->at(row)));
        else
            return QModelIndex();
    }
    
    int COverViewListModel::rowCount(const QModelIndex & parent) const {
        if (devices == NULL)
            return 0;
        
        if (!parent.isValid())
            return devices->count();
        
        return 0;
    }
    
    QVariant COverViewListModel::data(const QModelIndex & index, int role) const {
        if (devices == NULL)
            return QVariant();
        
        if (!index.isValid())
            return QVariant();
        
        if (index.row() >= devices->size())
            return QVariant();
        
        CDevice * data = (CDevice *)(index.internalPointer());
        
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0:
                    return data->name;
                case 1:
                    return data->enabled ? tr("enabled") : tr("disabled");
                case 2:
                    switch (data->type) {
                        case DT_ETH: return tr("Ethernet");
                        case DT_AIR: return tr("Wireless");
                        default:     break;
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
    
    QVariant COverViewListModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (devices == NULL)
            return QVariant();
        
        if (role != Qt::DisplayRole)
            return QVariant();
        
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return tr("Name");
                case 1:
                    return tr("Status");
                case 2:
                    return tr("Type");
                default:
                    break;
            }
        }
        return QVariant();
    }
};
