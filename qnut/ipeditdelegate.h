//
// C++ Interface: ipeditdelegate
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUTIPEDITDELEGATE_H
#define QNUTIPEDITDELEGATE_H

#include <QItemDelegate>

namespace qnut {
	class CIPEditDelegate : public QItemDelegate {
		Q_OBJECT
	public:
		CIPEditDelegate(QObject * parent = 0);
		~CIPEditDelegate();
		
		QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		
		void setEditorData(QWidget * editor, const QModelIndex & index) const;
		void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
	
		void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	};
};

#endif
