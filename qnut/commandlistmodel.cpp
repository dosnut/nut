//
// C++ Implementation: commandlistmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "commandlistmodel.h"

namespace qnut {
	CCommandListModel::CCommandListModel(QObject * parent) : QAbstractListModel(parent) {}
	
	void CCommandListModel::setList(QList<ToggleableCommand> & list) {
//		beginResetModel();
		m_Data = list;
		reset();
//		endResetModel();
	}
	
	int CCommandListModel::rowCount(const QModelIndex & /*parent*/) const {
//		if (parent.isValid())
//			return 0;
//		else
		return m_Data.size();
	}
	
	QVariant CCommandListModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid())
			return QVariant();
		
		if (index.row() >= m_Data.size())
			return QVariant();
	
		if (role == Qt::DisplayRole)
			return m_Data.at(index.row()).path;
		else
			return QVariant();
	}
	
	Qt::ItemFlags CCommandListModel::flags(const QModelIndex & index) const {
		if (!index.isValid())
			return Qt::ItemIsEnabled;
		
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
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
				}
				break;
			case Qt::CheckStateRole:
//				m_Data[index.row()].enabled = value.toInt() == Qt::Checked;
				m_Data[index.row()].enabled = value.toBool();
				break;
			default:
				return false;
			}
			return true;
		}
		return false;
	}
	
	QModelIndex CCommandListModel::appendRow(ToggleableCommand & command) {
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
