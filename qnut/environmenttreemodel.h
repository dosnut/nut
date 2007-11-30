//
// C++ Interface: environmenttreemodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_ENVIRONMENTTREEMODEL_H
#define QNUT_ENVIRONMENTTREEMODEL_H

#include <QAbstractItemModel>
#include <libnutclient/client.h>

namespace qnut {
	class CEnvironmentTreeModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		CEnvironmentTreeModel(libnutclient::CDevice * data, QObject * parent = 0);
		~CEnvironmentTreeModel();
		
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private:
		libnutclient::CDevice * m_Device;
	};
};

#endif
