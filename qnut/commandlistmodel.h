//
// C++ Interface: commandlistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_COMMANDLISTMODEL_H
#define QNUT_COMMANDLISTMODEL_H

#include <QAbstractListModel>
#include "common.h"

namespace qnut {
	/**
	 * @brief CCommandListModel provides an item model for an overview of a given the command list.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * The class provides all functions for an editalbe model specified in the Qt 4 documentation.
	 * This model supports displaying and editing of the listed commands.
	 */
	class CCommandListModel : public QAbstractListModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model with an empty list of commands.
		 * @param parent parent object
		 */
		CCommandListModel(QObject * parent = 0);
//		/// @brief Destroys the object.
//		~CommandListModel();
		
		/**
		 * @brief Caches the provided list
		 * @param list list of commands to cache
		 */
		void setList(QList<ToggleableCommand> & list);
		/// @brief returns the cached command list
		QList<ToggleableCommand> & cachedList() { return m_Data; }
		
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
		QModelIndex appendRow(ToggleableCommand & command);
		bool removeRows(int position, int rows, const QModelIndex & parent = QModelIndex());
	private:
		QList<ToggleableCommand> m_Data;
	};
}

#endif
