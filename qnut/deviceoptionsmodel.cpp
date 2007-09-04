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
#include "constants.h"
#include <QIcon>

namespace qnut {
    CDeviceOptionsModel::CDeviceOptionsModel(CDevice * data, QObject * parent) : QAbstractItemModel(parent) {
        device = data;
    }
    
    CDeviceOptionsModel::~CDeviceOptionsModel() {
        device = NULL;
    }
    
    int CDeviceOptionsModel::columnCount(const QModelIndex & parent) const {
        if (device == NULL)
            return 0;
        else
            return 3;
    }

    
    int CDeviceOptionsModel::rowCount(const QModelIndex & parent) const {
        if (device == NULL)
            return 0;
        
        if (!parent.isValid())
            return device->environments.count();
        else {
            QObject * parentData = (QObject *)(parent.internalPointer());
            if (parentData->parent() == device)
                return ((CEnvironment *)parentData)->interfaces.count();
            else
                return 0;
        }
    }
    
    QVariant CDeviceOptionsModel::data(const QModelIndex & index, int role) const {
        if (device == NULL)
            return QVariant();
        
        if (!index.isValid())
            return QVariant();
        
        QObject * data = (QObject *)(index.internalPointer());
        switch (index.column()) {
            case 0:
                if (data->parent() == device) {
                    switch (role) {
                        case Qt::DisplayRole:
                            return ((CEnvironment *)data)->name;
                        case Qt::DecorationRole:
                            return QIcon(UI_ICON_ENVIRONMENT);
                        default:
                            break;
                    }
                }
                
                if (data->parent()->parent() == device) {
                    switch (role) {
                        case Qt::DisplayRole:
                            return tr("IP-Address") + ": " +((CInterface *)data)->ip.toString() + "\n" +
                                tr("Netmask") + ": " + ((CInterface *)data)->netmask.toString() + "\n" +
                                tr("Gateway") + ": " + ((CInterface *)data)->gateway.toString();
                        case Qt::DecorationRole:
                            return QIcon(UI_ICON_INTERFACE);
                        default:
                            break;
                    }
                }
                break;
            case 1:
                if (role == Qt::DisplayRole) {
                    if (data->parent() == device) {
                        return ((CEnvironment *)data)->active ? tr("active") : QVariant();//tr("inactive");
                    }
                    
                    if (data->parent()->parent() == device) {
                        return ((CInterface *)data)->active ? tr("enabled") : tr("disabled");
                    }
                }
                break;
            case 2:
                if (role == Qt::DisplayRole) {
                    if (data->parent() == device) {
                        int configFlags = 0;
                        bool configUseMac = false;
                        foreach(SelectConfig config, ((CEnvironment *)data)->selectStatements) {
                            if (config.selected) {
                                configFlags = configFlags || config.flags;
                                configUseMac = configUseMac || config.useMac;
                            }
                        }
                        
                        QString result = tr("selected by") + " ";
                        switch (configFlags) {
                            case 3:
                                result += tr("essid") + ", " + tr("arp");
                                break;
                            case 2:
                                result += tr("essid");
                                break;
                            case 1:
                                result += tr("arp");
                                break;
                            default:
                                result += tr("user");
                                break;
                        }
                        return result + " " + (configUseMac ? tr("and MAC-Address") : "");
                    }
                    if (data->parent()->parent() == device) {
                        return ((CInterface *)data)->isStatic ? tr("static") : tr("dynamic");
                    }
                }
                break;
            default:
                break;
        }
        
        return QVariant();
    }
    
    Qt::ItemFlags CDeviceOptionsModel::flags(const QModelIndex & index) const {
        if (device == NULL)
            return 0;
        
        if (!index.isValid())
            return 0;
        
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    
    QVariant CDeviceOptionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (device == NULL)
            return QVariant();
        
        if (role != Qt::DisplayRole)
            return QVariant();
        
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return tr("Item");
                case 1:
                    return tr("Status");
                case 2:
                    return tr("Config");
                default:
                    break;
            }
        }
        return QVariant();
    }
    
    QModelIndex CDeviceOptionsModel::index(int row, int column, const QModelIndex & parent) const {
        if (device == NULL)
            return QModelIndex();
        
        if (!hasIndex(row, column, parent))
            return QModelIndex();
        
        
        if (!parent.isValid()) {
            if (row < device->environments.count())
                return createIndex(row, column, device->environments[row]);
        }
        else {
            QObject * parentData = (QObject *)(parent.internalPointer());
            if ((parentData->parent() == device) && (row < ((CEnvironment *)(parentData))->interfaces.count()))
                return createIndex(row, column, ((CEnvironment *)(parent.internalPointer()))->interfaces[row]);
        }
        
        return QModelIndex();
    }
    
    QModelIndex CDeviceOptionsModel::parent(const QModelIndex & index) const {
        if (device == NULL)
            return QModelIndex();
        
        if (!index.isValid())
            return QModelIndex();
        
        if (index.internalPointer() == NULL)
            return QModelIndex();
        
        QObject * parentData = ((QObject *)(index.internalPointer()))->parent();
        
        if (parentData->parent() == device)
            return createIndex(device->environments.indexOf((CEnvironment *)(parentData)), 0, (void *)(parentData));
        else
            return QModelIndex();
        
    }
};
