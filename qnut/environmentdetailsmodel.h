//
// C++ Interface: environmentdetailsmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_ENVIRONMENTDETAILSMODEL_H
#define QNUT_ENVIRONMENTDETAILSMODEL_H

#include <QAbstractItemModel>
#include <libnutclient/client.h>

namespace qnut {
	class CEnvironmentDetailsModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		CEnvironmentDetailsModel(libnutclient::CEnvironment * data = NULL, QObject * parent = 0);
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
