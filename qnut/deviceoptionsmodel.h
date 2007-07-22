//
// C++ Interface: deviceoptionsmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUTDEVICEOPTIONSMODEL_H
#define QNUTDEVICEOPTIONSMODEL_H

#include <QAbstractItemModel>
#include "libnut_cli.h"

namespace qnut {
    class CDeviceOptionsModel : public QAbstractItemModel {
        Q_OBJECT
    public:
        CDeviceOptionsModel(CDevice * data, QObject * parent = 0);
        ~CDeviceOptionsModel();
        
        QVariant data(const QModelIndex & index, int role) const;
        Qt::ItemFlags flags(const QModelIndex & index) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex & index) const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        int columnCount(const QModelIndex & parent = QModelIndex()) const;
    private:
        CDevice * device;
    };
}

#endif
