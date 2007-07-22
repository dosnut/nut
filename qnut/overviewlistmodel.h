//
// C++ Interface: overviewlistmodel
//
// Description: 
// Dies stellt die Spezifikation für das Model des ListView Steuerelemnts dar.
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_OVERVIEWLISTMODEL_H
#define QNUT_OVERVIEWLISTMODEL_H

#include <QAbstractListModel>
#include <QVariant>
#include "libnut_cli.h"

namespace qnut {

    class COverViewListModel : public QAbstractListModel {
        Q_OBJECT
    public:
        COverViewListModel(CDeviceList * deviceList, QObject * parent = 0);
        ~COverViewListModel();
        
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    private:
        CDeviceList * devices;
    };

};

#endif
