//
// C++ Interface: managedapmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_MANAGEDAPMODEL_H
#define QNUT_MANAGEDAPMODEL_H

#ifndef QNUT_NO_WIRELESS
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <libnutclient/client.h>

#define UI_MANAP_SSID   0
#define UI_MANAP_STATUS 1
#define UI_MANAP_ID     2
#define UI_MANAP_BSSID  3

namespace qnut {
	/**
	 * @brief CManagedAPModel provides an item model for an overview of the managed networks from a wpa_supplicant.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 * Additionally: A function to retrieve the cached data and a slot to force an update of the cached data are provided.
	 * 
	 * The model supports the display the following information in columns for each network:
	 *  - SSID
	 *  - current status (selected, enabled, disabled)
	 *  - ID (the unique id)
	 *  - BSSID (MAC address)
	 */
	class CManagedAPModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/// @brief Returns the cached list of managed networks.
		QList<libnutwireless::ShortNetworkInfo> cachedNetworks() const { return m_Networks; };
		
		/**
		 * @brief Creates the object and initializes the model according to the given wpa_supplicant object.
		 * @param wpaSupplicant pointer to a wpa_supplicant (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		CManagedAPModel(libnutwireless::CWpaSupplicant * wpaSupplicant = NULL, QObject * parent = 0);
		/// @brief Destroyes the object.
		~CManagedAPModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	public slots:
		/// @brief Updates the cached data.
		void updateNetworks();
	private:
		void setWpaSupplicant(libnutwireless::CWpaSupplicant * wpaSupplicant);
		QList<libnutwireless::ShortNetworkInfo> m_Networks;
		libnutwireless::CWpaSupplicant * m_Supplicant;
	};
	
	class CManagedAPProxyModel : public QSortFilterProxyModel {
		Q_OBJECT
	public:
		CManagedAPProxyModel(QObject * parent = 0);
		bool lessThan(const QModelIndex & left, const QModelIndex & right);
	};
}
#endif

#endif
