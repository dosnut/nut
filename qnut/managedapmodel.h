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
#include <libnutclient/client.h>

#define UI_MANAP_SSID   0
#define UI_MANAP_STATUS 1
#define UI_MANAP_ID     2
#define UI_MANAP_BSSID  3

namespace qnut {
	class CManagedAPModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		QList<libnutwireless::ShortNetworkInfo> cachedNetworks() const { return m_Networks; };
		
		CManagedAPModel(libnutwireless::CWpa_Supplicant * wpaSupplicant = NULL, QObject * parent = 0);
		~CManagedAPModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		
		void setWpaSupplicant(libnutwireless::CWpa_Supplicant * wpaSupplicant);
	public slots:
		void updateNetworks();
	private:
		QList<libnutwireless::ShortNetworkInfo> m_Networks;
		libnutwireless::CWpa_Supplicant * m_Supplicant;
	};
}

#endif
