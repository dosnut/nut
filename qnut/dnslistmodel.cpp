//
// C++ Implementation: dnslistmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dnslistmodel.h"

namespace qnut {
	CDNSListModel::CDNSListModel(QList<QHostAddress> * dnsList, QObject * parent) : QAbstractListModel(parent), dnsList(dnsList) {
	}
	
	CDNSListModel::~CDNSListModel() {
		dnsList = NULL;
	}
	
	int CDNSListModel::rowCount(const QModelIndex &) const {
		if (dnsList)
			return dnsList->size();
		else
			return 0;
	}
	
	QVariant CDNSListModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid())
			return QVariant();
		
		if (index.row() >= dnsList->size())
			return QVariant();
	
		if (role == Qt::DisplayRole)
			return dnsList->at(index.row()).toString();
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
			dnsList->replace(index.row(), QHostAddress(value.toString()));
			emit dataChanged(index, index);
			return true;
		}
		return false;
	}
};
