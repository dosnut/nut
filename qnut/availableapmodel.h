//
// C++ Interface: availableapmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_AVAILABLEAPMODEL_H
#define QNUT_AVAILABLEAPMODEL_H

#include <QAbstractItemModel>
#include <libnutclient/client.h>

#define UI_AVLAP_SSID    0
#define UI_AVLAP_KEYMGMT 1
#define UI_AVLAP_ENC     2
#define UI_AVLAP_BSSID   3
#define UI_AVLAP_CHANNEL 4
#define UI_AVLAP_QUALITY 5
#define UI_AVLAP_LEVEL   6

namespace qnut {
	class CAvailableAPModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		QList<libnutwireless::ScanResult> cachedScans() const { return m_Scans; };
		
		CAvailableAPModel(libnutwireless::CWpa_Supplicant * data = NULL, QObject * parent = 0);
		~CAvailableAPModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		
		void setWpaSupplicant(libnutwireless::CWpa_Supplicant * wpaSupplicant);
	private slots:
		void updateScans();
	private:
		libnutwireless::CWpa_Supplicant * m_Supplicant;
		QList<libnutwireless::ScanResult> m_Scans;
	};
}

#endif
