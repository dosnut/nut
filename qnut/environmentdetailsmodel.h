//
// C++ Interface: environmentdetailsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_ENVIRONMENTDETAILSMODEL_H
#define QNUT_ENVIRONMENTDETAILSMODEL_H

#include <QAbstractItemModel>
#include <libnutclient/client.h>

namespace qnut {
	/**
	 * @brief CInterfaceDetailsModel provides an item model for an overview of the select statements from an environment.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 * 
	 * The model supports the display the following information in columns for each select statement:
	 *  - statement (all (and block), at least one (or block), by user, by arp, by ssid)
	 *  - value (ARP statement, SSID)
	 */
	class CEnvironmentDetailsModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model according to the given environment.
		 * @param data pointer to an environment (if NULL nothing is displayed)
		 */
		CEnvironmentDetailsModel(libnutclient::CEnvironment * data = NULL, QObject * parent = 0);
		/// @brief Destroyes the object.
		~CEnvironmentDetailsModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private:
		void fillParentRules(quint32 start = 0);
		
		libnutclient::CEnvironment * m_Environment;
		libnutcommon::SelectConfig m_SelectConfig;
		QVector<quint32> m_ParentRules;
	};
}

#endif
