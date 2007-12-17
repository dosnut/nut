//
// C++ Implementation: managedapmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QIcon>
#include "managedapmodel.h"
#include "constants.h"

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;
	
	CManagedAPModel::CManagedAPModel(CWpaSupplicant * wpaSupplicant, QObject * parent) : QAbstractItemModel(parent) {
		setWpaSupplicant(wpaSupplicant);
	}
	
	CManagedAPModel::~CManagedAPModel() {
		m_Supplicant = NULL;
	}
	
	void CManagedAPModel::setWpaSupplicant(CWpaSupplicant * wpaSupplicant) {
		if (m_Supplicant == wpaSupplicant)
			return;
		
		m_Supplicant = wpaSupplicant;
		if (m_Supplicant) {
			updateNetworks();
			connect(m_Supplicant, SIGNAL(networkListUpdated()), this, SLOT(updateNetworks()));
		}
	}
	
	void CManagedAPModel::updateNetworks() {
		emit layoutAboutToBeChanged();
		m_Networks = m_Supplicant->listNetworks();
		emit layoutChanged();
	}
	
	int CManagedAPModel::columnCount(const QModelIndex &) const {
		return 4;
	}
	
	int CManagedAPModel::rowCount(const QModelIndex & parent) const {
		if (!parent.isValid())
			return m_Networks.count();
		else
			return 0;
	}
	
	QVariant CManagedAPModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid())
			return QVariant();
		
		if ((role == Qt::DecorationRole) && (index.column() == 0))
			return QIcon(m_Networks[index.row()].adhoc ? UI_ICON_ADHOC : UI_ICON_AIR_ACTIVATED);
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case UI_MANAP_ID:
			return QString::number(index.row());
		case UI_MANAP_SSID:
			return m_Networks[index.row()].ssid;
		case UI_MANAP_BSSID:
			if (m_Networks[index.row()].bssid.zero())
				return tr("any");
			else
				return m_Networks[index.row()].bssid.toString();
		case UI_MANAP_STATUS:
			//strange "flags"
			switch (m_Networks[index.row()].flags) {
			case NF_CURRENT:
				return tr("selected");
			case NF_DISABLED:
				return tr("disabled");
			default:
				return tr("enabled");
			}
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CManagedAPModel::flags(const QModelIndex & index) const {
		if ((m_Networks.isEmpty()) || (!index.isValid()))
			return 0;
		
		return (m_Networks[index.row()].flags == NF_CURRENT) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::ItemIsSelectable;
	}
	
	QVariant CManagedAPModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case UI_MANAP_ID:
				return tr("ID");
			case UI_MANAP_SSID:
				return tr("SSID");
			case UI_MANAP_BSSID:
				return tr("BSSID");
			case UI_MANAP_STATUS:
				return tr("status");
			default:
				break;
			}
		}
		return QVariant();
	}
	
	QModelIndex CManagedAPModel::index(int row, int column, const QModelIndex & parent) const {
		//qDebug() << "index(" << row << column << ')';
		if (m_Networks.isEmpty() || parent.isValid() || row >= m_Networks.count())
			return QModelIndex();
		else
			return createIndex(row, column, m_Networks[row].id);
	}
	
	QModelIndex CManagedAPModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
};
