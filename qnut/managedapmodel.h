//
// C++ Interface: managedapmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_MANAGEDAPMODEL_H
#define QNUT_MANAGEDAPMODEL_H

#include <QAbstractItemModel>
#include <libnut/libnut_cli.h>

namespace qnut {
	class CManagedAPModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		CManagedAPModel(libnutws::CWpa_Supplicant * wpaSupplicant = NULL, QObject * parent = 0);
		~CManagedAPModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		
		void setWpaSupplicant(libnutws::CWpa_Supplicant * wpaSupplicant);
	public slots:
		void reloadNetworks();
	private:
		libnutws::CWpa_Supplicant * supplicant;
		QList<libnutws::wps_network> networks;
	};
}

#endif
