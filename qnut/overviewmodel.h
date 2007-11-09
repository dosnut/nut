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
#ifndef QNUT_OVERVIEWMODEL_H
#define QNUT_OVERVIEWMODEL_H

#include <QAbstractItemModel>
#include <libnut/libnut_cli.h>

namespace qnut {
	class COverViewModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		COverViewModel(libnut::CDeviceManager * deviceManager, QObject * parent = 0);
		~COverViewModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column , const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private:
		libnut::CDeviceList * devices;
	private slots:
		void deviceAdded(libnut::CDevice * device);
		void deviceRemoved(libnut::CDevice * device);
	};

};

#endif
