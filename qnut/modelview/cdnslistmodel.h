//
// C++ Interface: dnslistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_DNSLISTMODEL_H
#define QNUT_DNSLISTMODEL_H

#include <QAbstractListModel>
#include <QHostAddress>

namespace qnut {
	/**
	 * @brief CDNSListModel provides an item model for an overview of a given the dns server list.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * The class provides all functions for an editalbe model specified in the Qt 4 documentation.
	 *
	 * The model supports displaying and editing of the listed host addresses.
	 */
	class CDNSListModel : public QAbstractListModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model according to the given list of host addresses.
		 * @param dnsList pointer to a list of host addresses (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		CDNSListModel(QList<QHostAddress> * dnsList, QObject * parent = 0);
		/// @brief Destroyes the object.
		~CDNSListModel();

		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
		QModelIndex appendRow(QHostAddress address);
		bool removeRows(int position, int rows, const QModelIndex & parent = QModelIndex());
	private:
		QList<QHostAddress> * m_DNSList;
	};
}

#endif
