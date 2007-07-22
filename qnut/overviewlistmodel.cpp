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
#include "overviewlistmodel.h"

namespace qnut {
    COverViewListModel::COverViewListModel(CDeviceList * deviceList, QObject * parent) : QAbstractListModel(parent) {
        devices = deviceList;
        // vllt sollte hier die deviceliste gefüllt werden wenn nicht vorher getan
    }
    
    COverViewListModel::~COverViewListModel() {
        devices = NULL;
    }
    
    int COverViewListModel::rowCount(const QModelIndex & parent) const {
        if (devices == NULL)
            return 0;
        else
            return devices->count();
    }
    
    QVariant COverViewListModel::data(const QModelIndex & index, int role) const {
        if (devices == NULL)
            return QVariant();
        
        if (!index.isValid())
            return QVariant();
        
        if (index.row() >= devices->size())
            return QVariant();
        
        if (role != Qt::DisplayRole)
            return QVariant();
        
        if (index.column() == 0)
            return QVariant(devices->value(index.row())->name);
        else if (index.column() == 1)
            return QVariant(devices->value(index.row())->enabled ? tr("enabled") : tr("disabled") );
        else
            return QVariant();
    }
    
    QVariant COverViewListModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (devices == NULL)
            return QVariant();
        
        if (role != Qt::DisplayRole)
            return QVariant();
        
        if (orientation == Qt::Horizontal)
            switch (section) {
                case 0:
                    return QVariant(tr("Name"));
                case 1:
                    return QVariant(tr("status"));
                default:
                    return QVariant();
            }
        else
            return QVariant();
    }
};
