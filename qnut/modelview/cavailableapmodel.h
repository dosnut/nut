//
// C++ Interface: availableapmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_AVAILABLEAPMODEL_H
#define QNUT_AVAILABLEAPMODEL_H

#ifndef NUT_NO_WIRELESS
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <libnutwireless/hwtypes.h>

namespace libnutwireless {
	class CWirelessHW;
}

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
		/**
		 * @brief Creates the object and initializes the model according to the given wpa_supplicant object.
		 * @param wpaSupplicant pointer to a wpa_supplicant (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		explicit CAvailableAPModel(libnutwireless::CWirelessHW* data, QObject* parent = nullptr);

		/// @brief Returns a cached scan result by a given model index (or nullptr if index is not valid)
		libnutwireless::ScanResult const* scanResultByModelIndex(const QModelIndex& index) const;

		/// @brief Returns a list of pointers to scan results that match the provided SSID
		QList<libnutwireless::ScanResult const*> scanResultListBySSID(QString ssid) const;

		QVariant data(const QModelIndex & index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex & index) const override;
		bool hasChildren(const QModelIndex & parent = QModelIndex()) const override;
		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		int columnCount(const QModelIndex & parent = QModelIndex()) const override;

	signals:
		void cachedScansUpdated();

	private slots:
		void updateScans();

	private:
		/// @brief Returns a cached scan result id by a given model index
		int scanResultIdByModelIndex(const QModelIndex & index) const;

	private:
		libnutwireless::CWirelessHW* const m_WirelessAccess = nullptr;

		QList<libnutwireless::ScanResult> m_Scans;
		QHash<QString, QList<int>> m_GroupedScans;
		QList<QString> m_SSIDs;
	};

	class CAvailableAPProxyModel : public QSortFilterProxyModel {
		Q_OBJECT
	public:
		explicit CAvailableAPProxyModel(QObject* parent = 0);

	protected:
		bool lessThan(const QModelIndex & left, const QModelIndex & right) const override;
	};
}
#endif

#endif
