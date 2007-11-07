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
	CDNSListModel::CDNSListModel(QList<QHostAddress> * dnsList, QObject *parent) : QAbstractListModel(parent), data(dnsList) {
	}
	
	CDNSListModel::~CDNSListModel() {
		data = NULL;
	}
	
	int CDNSListModel::rowCount(const QModelIndex &parent) const {
		if (data)
			return data->size();
		else
			return 0;
	}
	
	QVariant CDNSListModel::data(const QModelIndex &index, int role) const {
		if (!index.isValid())
			return QVariant();
		
		if (index.row() >= data->size())
			return QVariant();
	
		if (role == Qt::DisplayRole)
			return data->at(index.row()).toString();
		else
			return QVariant();
	}
	
	Qt::ItemFlags CDNSListModel::flags(const QModelIndex &index) const {
		if (!index.isValid())
			return Qt::ItemIsEnabled;
		
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	}
	
	bool CDNSListModel::setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) {
		if (index.isValid() && role == Qt::EditRole) {
			data->replace(index.row(), QHostAddress(value.toString()));
			emit dataChanged(index, index);
			return true;
		}
		return false;
	}
};
