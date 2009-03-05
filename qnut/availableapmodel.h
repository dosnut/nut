//
// C++ Interface: availableapmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_AVAILABLEAPMODEL_H
#define QNUT_AVAILABLEAPMODEL_H

#ifndef QNUT_NO_WIRELESS
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <libnutclient/client.h>

#define UI_AVLAP_SSID    0
#define UI_AVLAP_KEYMGMT 1
#define UI_AVLAP_ENC     2
#define UI_AVLAP_BSSID   3
#define UI_AVLAP_CHANNEL 4
#define UI_AVLAP_QUALITY 5
#define UI_AVLAP_LEVEL   6

namespace qnut {
	/**
	 * @brief CAvailableAPModel provides an item model for an overview of the available networks (scan results) from a wpa_supplicant.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 * Additionally: A function to retrieve the cached data is provided.
	 * 
	 * The model supports the display the following information in columns for each scan result:
	 *  - SSID
	 *  - key management
	 *  - encryption ciphers (group and pairwise)
	 *  - BSSID (MAC address)
	 *  - channel
	 *  - signal quality
	 *  - signal level
	 */
	class CAvailableAPModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/// @brief Returns the cached list of scan results.
		QList<libnutwireless::ScanResult> cachedScans() const { return m_Scans; };
		
		/**
		 * @brief Creates the object and initializes the model according to the given wpa_supplicant object.
		 * @param wpaSupplicant pointer to a wpa_supplicant (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		CAvailableAPModel(libnutwireless::CWpaSupplicant * data = NULL, QObject * parent = 0);
		/// @brief Destroyes the object.
		~CAvailableAPModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private slots:
		void updateScans();
	private:
		void setWpaSupplicant(libnutwireless::CWpaSupplicant * wpaSupplicant);
		libnutwireless::CWpaSupplicant * m_Supplicant;
		QList<libnutwireless::ScanResult> m_Scans;
	};
	
	class CAvailableAPProxyModel : public QSortFilterProxyModel {
		Q_OBJECT
	public:
		CAvailableAPProxyModel(QObject * parent = 0);
		bool lessThan(const QModelIndex & left, const QModelIndex & right);
	};
}
#endif

#endif
