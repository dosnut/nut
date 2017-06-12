//
// C++ Interface: managedapmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_MANAGEDAPMODEL_H
#define QNUT_MANAGEDAPMODEL_H

#ifndef NUT_NO_WIRELESS
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <libnutwireless/wpa_supplicant.h>

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
		/**
		 * @brief Creates the object and initializes the model according to the given wpa_supplicant object.
		 * @param wpaSupplicant pointer to a wpa_supplicant (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		explicit CManagedAPModel(libnutwireless::CWpaSupplicant* wpaSupplicant = nullptr, QObject* parent = nullptr);

		/// @brief Returns the cached list of managed networks.
		QList<libnutwireless::ShortNetworkInfo> const& cachedNetworks() const { return m_Networks; }

		libnutwireless::ShortNetworkInfo const* networkInfoByModelIndex(QModelIndex const& index) const;
		libnutwireless::ShortNetworkInfo const* currentNetworkInfo() const { return m_CurrentNetwork; }

		QVariant data(const QModelIndex & index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex & index) const override;
		bool hasChildren(const QModelIndex & parent = QModelIndex()) const override;
		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		int columnCount(const QModelIndex & parent = QModelIndex()) const override;

	public slots:
		/// @brief Updates the cached data.
		void updateNetworks();

	private:
		libnutwireless::CWpaSupplicant * const m_Supplicant = nullptr;
		QList<libnutwireless::ShortNetworkInfo> m_Networks;
		libnutwireless::ShortNetworkInfo* m_CurrentNetwork = nullptr;
	};

	class CManagedAPProxyModel : public QSortFilterProxyModel {
		Q_OBJECT
	public:
		explicit CManagedAPProxyModel(QObject* parent = nullptr);
		bool lessThan(const QModelIndex & left, const QModelIndex & right) const override;
	};
}
#endif

#endif
