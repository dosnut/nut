//
// C++ Interface: ipeditdelegate
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_IPEDITDELEGATE_H
#define QNUT_IPEDITDELEGATE_H

#include <QStyledItemDelegate>

namespace qnut {
	/**
	 * @brief CIPEditDelegate provides an item delegate for editing ip addresses.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * The class provides all functions for a simple item delegate specified in the Qt 4 documentation.
	 *
	 * As editor QLineEdit with an input mask for ip addresses is used.
	 */
	class CIPEditDelegate : public QStyledItemDelegate {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the delegate.
		 * @param parent parent object
		 */
		explicit CIPEditDelegate(QObject* parent = nullptr);

		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		void setEditorData(QWidget* editor, const QModelIndex& index) const override;
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	};
}

#endif
