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
#include "deviceoptionsmodel.h"

namespace qnut {
    CDeviceOptionsModel::CDeviceOptionsModel(CDevice * data, QObject * parent) : QAbstractItemModel(parent) {
        device = data;
    }
    
    CDeviceOptionsModel::~CDeviceOptionsModel() {
        device = NULL;
    }
    
    QVariant CDeviceOptionsModel::data(const QModelIndex & index, int role) const {
        if (device == NULL)
            return QVariant();
        
        if (!index.isValid())
            return QVariant();
        
        if (role != Qt::DisplayRole)
            return QVariant();
        
        switch (index.column()) {
            case 0:
                return QVariant(((CEnvironment *)(index.internalPointer()))->properties.name);
            case 1:
                return QVariant(((CInterface *)(index.internalPointer()))->properties.ip.toString());
            case 2:
                switch (index.row()) {
                case 0:
                    return QVariant(tr("Netmask: ") + ((CInterface *)(index.internalPointer()))->properties.netmask.toString());
                case 1:
                    return QVariant(tr("Gateway: ") + ((CInterface *)(index.internalPointer()))->properties.gateway.toString());
                default:
                    return QVariant();
                }
            default:
                return QVariant();
        }
    }
    
    Qt::ItemFlags CDeviceOptionsModel::flags(const QModelIndex & index) const {
        if (device == NULL)
            return 0;
        
        if (!index.isValid())
            return 0;
        
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    
    QVariant CDeviceOptionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
        return QVariant();
    }
    
    QModelIndex CDeviceOptionsModel::index(int row, int column, const QModelIndex & parent) const {
        if (device == NULL)
            return QModelIndex();
    
        if (!hasIndex(row, column, parent))
            return QModelIndex();
        
        if ((column == 0) && (row < device->environments.count()))
            return createIndex(row, column, device->environments[row]);
        else if ((column == 1) && (row < ((CEnvironment *)(parent.internalPointer()))->interfaces.count()))
            return createIndex(row, column, ((CEnvironment *)(parent.internalPointer()))->interfaces[row]);
        else if ((column == 2) && (row < 2))
            return createIndex(row, column, parent.internalPointer());
        else
            return QModelIndex();
    }
    
    QModelIndex CDeviceOptionsModel::parent(const QModelIndex & index) const {
        if (device == NULL)
            return QModelIndex();
        
        if (!index.isValid())
            return QModelIndex();
        
        switch (index.column()) {
            case 1: {
                CInterface * currentInterface = (CInterface *)(index.internalPointer());
                CEnvironment * parentEnvironment = (CEnvironment *)(currentInterface->parent());
                
                return createIndex(device->environments.indexOf(parentEnvironment), 0, (void *)parentEnvironment);
            }
            case 2: {
                CInterface * parentInterface = (CInterface *)(index.internalPointer());
                CEnvironment * parentEnvironment = (CEnvironment *)(parentInterface->parent());
                
                return createIndex(parentEnvironment->interfaces.indexOf(parentInterface), 1, (void *)parentInterface);
            }
        default:
            return QModelIndex();
        }
    }
    
    int CDeviceOptionsModel::rowCount(const QModelIndex & parent) const {
        if (device == NULL)
            return 0;
        
        if (!parent.isValid())
            return device->environments.count();
        
        switch (parent.column()) {
            case 0:
                return ((CEnvironment *)(parent.internalPointer()))->interfaces.count();
            case 1:
                return 2;
            default:
                return 0;
        }
    }
    
    int CDeviceOptionsModel::columnCount(const QModelIndex & parent) const {
        if ((device == NULL) || (!parent.isValid()))
            return 0;
        else
            return 1;
    }
};
