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
	 * The class provides all functions for an editable model specified in the Qt 4 documentation.
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
		explicit CDNSListModel(QList<QHostAddress> const& dnsList, QObject* parent = nullptr);

		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		QVariant data(const QModelIndex & index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
		bool removeRows(int position, int rows, const QModelIndex & parent = QModelIndex()) override;

		QModelIndex appendRow(QHostAddress address);
		QList<QHostAddress> const& list() const { return m_DNSList; }
	private:
		QList<QHostAddress> m_DNSList;
	};
}

#endif
