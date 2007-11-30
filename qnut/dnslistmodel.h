//
// C++ Interface: dnslistmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_DNSLISTMODEL_H
#define QNUT_DNSLISTMODEL_H

#include <QAbstractListModel>
#include <QHostAddress>

namespace qnut {
	class CDNSListModel : public QAbstractListModel {
		Q_OBJECT
	public:
		CDNSListModel(QList<QHostAddress> * dnsList, QObject * parent = 0);
		~CDNSListModel();
		
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
		bool appendRow(QHostAddress address);
		bool removeRows(int position, int rows, const QModelIndex & parent = QModelIndex());
	private:
		QList<QHostAddress> * m_DNSList;
	};
};

#endif
