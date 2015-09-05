//
// C++ Implementation: commandlistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "modelview/ccommandlistmodel.h"

namespace qnut {
	CCommandListModel::CCommandListModel(QObject * parent) : QAbstractListModel(parent) {}

	void CCommandListModel::setList(QList<ToggleableCommand> & list) {
#if QT_VERSION >= 0x040600
		beginResetModel();
		m_Data = list;
		endResetModel();
#else
		m_Data = list;
		reset();
#endif
	}

	void CCommandListModel::setAllEnabled(bool value) {
		for (int i = 0; i < m_Data.size(); ++i)
			m_Data[i].enabled = value;

		emit dataChanged(index(0), index(m_Data.size()-1));
	}

	int CCommandListModel::rowCount(const QModelIndex & parent) const {
		if (parent.isValid())
			return 0;
		else
			return m_Data.size();
	}

	QVariant CCommandListModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid() || index.row() >= m_Data.size())
			return QVariant();

		switch (role) {
		case Qt::CheckStateRole:
			return m_Data.at(index.row()).enabled ? Qt::Checked : Qt::Unchecked;
		case Qt::EditRole:
		case Qt::DisplayRole:
			return m_Data.at(index.row()).path;
		default:
			return QVariant();
		}
	}

	Qt::ItemFlags CCommandListModel::flags(const QModelIndex & index) const {
		if (index.isValid())
			return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
		else
			return Qt::ItemIsEnabled;

	}

	bool CCommandListModel::setData(const QModelIndex & index, const QVariant & value, int role) {
		if (index.isValid()) {
			switch(role) {
			case Qt::EditRole: {
					QString newPath = value.toString();
					if (newPath.isEmpty())
						return false;
					m_Data[index.row()].path = newPath;
					emit dataChanged(index, index);
				return true;
				}
			case Qt::CheckStateRole:
				m_Data[index.row()].enabled = value.toInt() == Qt::Checked;
//				m_Data[index.row()].enabled = value.toBool();
				emit dataChanged(index, index);
				return true;
			default:
				return false;
			}
		}
		return false;
	}

	QModelIndex CCommandListModel::appendRow(ToggleableCommand command) {
		beginInsertRows(QModelIndex(), m_Data.size(), m_Data.size());

		m_Data << command;

		endInsertRows();
		return index(m_Data.size()-1);
	}

	bool CCommandListModel::removeRows(int position, int rows, const QModelIndex & parent) {
		beginRemoveRows(parent, position, position+rows-1);

		for (int i = position; i < position+rows; ++i)
			m_Data.removeAt(i);

		endRemoveRows();
		return true;
	}
}
