//
// C++ Interface: environmenttreemodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_ENVIRONMENTTREEMODEL_H
#define QNUT_ENVIRONMENTTREEMODEL_H

#include <QAbstractItemModel>

namespace libnutclient {
	class CDevice;
}

namespace qnut {
	/**
	 * @brief CEnvironmentTreeModel provides an item model for an overview of the environments and interfaces from a device.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 *
	 * The model supports the display the following information in columns for each item:
	 *  - item (environment name or interface number)
	 *  - current status (environment: active; interface: off, static, dynamic, zeroconf)
	 *  - ip address (only valid for interfaces)
	 */
	class CEnvironmentTreeModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model according to the given device.
		 * @param data pointer to a device (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		explicit CEnvironmentTreeModel(libnutclient::CDevice* data, QObject* parent = nullptr);

		QVariant data(const QModelIndex & index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex & index) const override;
		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		int columnCount(const QModelIndex & parent = QModelIndex()) const override;

	private slots:
		void layoutChangedDefault();

	private:
		libnutclient::CDevice* const m_Device;
	};
}

#endif
