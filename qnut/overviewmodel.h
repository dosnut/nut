//
// C++ Interface: overviewlistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_OVERVIEWMODEL_H
#define QNUT_OVERVIEWMODEL_H

#include <QAbstractItemModel>
#include <libnutclient/client.h>

namespace qnut {
	/**
	 * @brief COverViewModel provides an item model for an overview of the devices from a device manager.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 * 
	 * The model supports the display the following information in columns for each device:
	 *  - name
	 *  - current status (deactivated, activated, got carrier, unconfigured, up)
	 *  - type (wireless, ethernet)
	 *  - name of the active environment
	 *  - current IP address
	 *  - current network ("Local" for ethernet)
	 */
	class COverViewModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model according to the given device manager.
		 * @param deviceManager pointer to a device manager (if NULL nothing is displayed)
		 */
		COverViewModel(libnutclient::CDeviceManager * deviceManager, QObject * parent = 0);
		/// @brief Destroyes the object.
		~COverViewModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column , const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private:
		const libnutclient::CDeviceList * m_Devices;
	private slots:
		void deviceAdded(libnutclient::CDevice * device);
		void deviceRemoved(libnutclient::CDevice * device);
	};

};

#endif
