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

#define DEV_OPMOD_ITEM    0
#define DEV_OPMOD_STATUS  1
#define DEV_OPMOD_IP      2
#define DEV_OPMOD_CONFIG  5
#define DEV_OPMOD_NETMASK 3
#define DEV_OPMOD_GATEWAY 4

namespace qnut {
    CDeviceOptionsModel::CDeviceOptionsModel(CDevice * data, QObject * parent) : QAbstractItemModel(parent) {
        device = data;
    }
    
    CDeviceOptionsModel::~CDeviceOptionsModel() {
        device = NULL;
    }
    /*
    inline static QString CDeviceOptionsModel::getInterfaceName(CInterface * interface) {
            return tr("IP-Address") + ": " + (interface->active ? interface->ip.toString()      : tr("no dynamic")) + "\n" +
                   tr("Netmask")    + ": " + (interface->active ? interface->netmask.toString() : tr("no dynamic")) + "\n" +
                   tr("Gateway")    + ": " + (interface->active ? interface->gateway.toString() : tr("no dynamic"));
    }*/
    
    int CDeviceOptionsModel::columnCount(const QModelIndex & parent) const {
        if (device == NULL)
            return 0;
        else
            return 6;
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
            case DEV_OPMOD_ITEM:
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
                        return '#' + QString::number(((CEnvironment *)(data->parent()))->interfaces.indexOf((CInterface *)data));
                        //return getInterfaceName((CInterface *)data);
                    case Qt::DecorationRole:
                        return QIcon(UI_ICON_INTERFACE);
                    default:
                        break;
                    }
                }
                break;
            case DEV_OPMOD_STATUS:
                if (role == Qt::DisplayRole) {
                    if (data->parent() == device) {
                        return ((CEnvironment *)data)->active ? tr("active") : QVariant();
                    }
                    
                    if (data->parent()->parent() == device) {
                        return ((CInterface *)data)->active ? tr("enabled") : tr("disabled");
                    }
                }
                break;
            case DEV_OPMOD_CONFIG:
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
            case DEV_OPMOD_IP:
                if (role == Qt::DisplayRole) {
                    if (data->parent() == device) {
                        return QString('-');
                    }
                    
                    if (data->parent()->parent() == device) {
                        return (((CInterface *)data)->isStatic || ((CInterface *)data)->active) ? ((CInterface *)data)->ip.toString() : tr("not assigned");
                    }
                }
                break;
            case DEV_OPMOD_NETMASK:
                if (role == Qt::DisplayRole) {
                    if (data->parent() == device) {
                        return QString('-');
                    }
                    
                    if (data->parent()->parent() == device) {
                        return (((CInterface *)data)->isStatic || ((CInterface *)data)->active) ? ((CInterface *)data)->netmask.toString() : tr("not assigned");
                    }
                }
                break;
            case DEV_OPMOD_GATEWAY:
                if (role == Qt::DisplayRole) {
                    if (data->parent() == device) {
                        return QString('-');
                    }
                    
                    if (data->parent()->parent() == device) {
                        return (((CInterface *)data)->isStatic || ((CInterface *)data)->active) ? ((CInterface *)data)->gateway.toString() : tr("not assigned");
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
            case DEV_OPMOD_ITEM:
                return tr("Item");
                //break;
            case DEV_OPMOD_STATUS:
                return tr("Status");
            case DEV_OPMOD_CONFIG:
                return tr("Config");
            case DEV_OPMOD_IP:
                return tr("IP-Address");
            case DEV_OPMOD_NETMASK:
                return tr("Netmask");
            case DEV_OPMOD_GATEWAY:
                return tr("Gateway");
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
