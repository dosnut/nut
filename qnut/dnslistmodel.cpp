//
// C++ Implementation: dnslistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "dnslistmodel.h"

namespace qnut {
	CDNSListModel::CDNSListModel(QList<QHostAddress> * dnsList, QObject * parent) : QAbstractListModel(parent), m_DNSList(dnsList) {
	}
	
	CDNSListModel::~CDNSListModel() {
		m_DNSList = NULL;
	}
	
	int CDNSListModel::rowCount(const QModelIndex &) const {
		if (m_DNSList)
			return m_DNSList->size();
		else
			return 0;
	}
	
	QVariant CDNSListModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid())
			return QVariant();
		
		if (index.row() >= m_DNSList->size())
			return QVariant();
	
		if (role == Qt::DisplayRole)
			return m_DNSList->at(index.row()).toString();
		else
			return QVariant();
	}
	
	Qt::ItemFlags CDNSListModel::flags(const QModelIndex & index) const {
		if (!index.isValid())
			return Qt::ItemIsEnabled;
		
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	}
	
	bool CDNSListModel::setData(const QModelIndex & index, const QVariant & value, int role) {
		if (index.isValid() && role == Qt::EditRole) {
			QHostAddress address = QHostAddress(value.toString());
			if (address.isNull())
				return false;
			m_DNSList->replace(index.row(), QHostAddress(value.toString()));
			emit dataChanged(index, index);
			return true;
		}
		return false;
	}
	
	QModelIndex CDNSListModel::appendRow(QHostAddress address) {
		beginInsertRows(QModelIndex(), m_DNSList->size(), m_DNSList->size());
		
		*m_DNSList << address;
		
		endInsertRows();
		return index(m_DNSList->size()-1);
	}
	
	bool CDNSListModel::removeRows(int position, int rows, const QModelIndex &) {
		beginRemoveRows(QModelIndex(), position, position+rows-1);
	
		for (int row = 0; row < rows; ++row) {
			m_DNSList->removeAt(position);
		}
	
		endRemoveRows();
		return true;
	}
}
