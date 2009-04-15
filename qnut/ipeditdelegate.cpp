//
// C++ Implementation: ipeditdelegate
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QAbstractItemModel>
#include <QHostAddress>
#include <QEvent>
#include <QLineEdit>
#include <QDebug>
#include "ipeditdelegate.h"

namespace qnut {
	CIPEditDelegate::CIPEditDelegate(QObject * parent) : QStyledItemDelegate(parent) {
	}
	
	QWidget * CIPEditDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex &) const {
		QLineEdit * editor = new QLineEdit(parent);
		editor->setFrame(false);
		editor->setInputMask("000.000.000.000;_");
		return editor;
	}
	
	void CIPEditDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		QString value = index.data().toString();
		
		QLineEdit * lineEdit = static_cast<QLineEdit *>(editor);
		lineEdit->setText(value);
	}
	
	void CIPEditDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		QLineEdit * lineEdit = static_cast<QLineEdit *>(editor);
		QString value = lineEdit->text();
		
		model->setData(index, value, Qt::EditRole);
	}
}
