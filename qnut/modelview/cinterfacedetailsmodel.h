//
// C++ Interface: deviceoptionsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_INTERFACEDETAILSMODEL_H
#define QNUT_INTERFACEDETAILSMODEL_H

#include <QAbstractItemModel>

namespace libnutclient {
	class CInterface;
}

namespace qnut {
	/**
	 * @brief CInterfaceDetailsModel provides an item model for an overview of the details from an interface.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 *
	 * The model supports the display the following information in rows for the given interface:
	 *  - type ((user)static, dynamic, zeroconf)
	 *  - ip address
	 *  - netmask
	 *  - gateway
	 *  - list of dns servers
	 */
	class CInterfaceDetailsModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model according to the given interface.
		 * @param data pointer to an interface (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		CInterfaceDetailsModel(libnutclient::CInterface * data = NULL, QObject * parent = 0);
		/// @brief Destroyes the object.
		~CInterfaceDetailsModel();

		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private:
		libnutclient::CInterface * m_Interface;
	};
}

#endif
