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
	 * @brief CCommandListModel provides a simple item list model for a given the command list.
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
		explicit CCommandListModel(QObject* parent = nullptr);

		/**
		 * @brief Caches the provided list
		 * @param list list of commands to cache
		 */
		void setList(QList<ToggleableCommand> const& list);

		/// @brief returns the cached command list
		QList<ToggleableCommand>& cachedList() { return m_Data; }

		/**
		 * @brief sets enabled value for all commands in the cached list
		 * @param list list of commands to cache
		 */
		void setAllEnabled(bool value);

		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		QVariant data(const QModelIndex & index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
		bool removeRows(int position, int rows, const QModelIndex & parent = QModelIndex()) override;

		QModelIndex appendRow(ToggleableCommand command = ToggleableCommand());
	private:
		QList<ToggleableCommand> m_Data;
	};
}

#endif
