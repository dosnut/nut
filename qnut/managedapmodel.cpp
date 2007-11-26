//
// C++ Implementation: managedapmodel
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QIcon>
#include "managedapmodel.h"
#include "constants.h"
#include <QDebug>
namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;
	
	CManagedAPModel::CManagedAPModel(CWpa_Supplicant * wpaSupplicant, QObject * parent) : QAbstractItemModel(parent) {
		setWpaSupplicant(wpaSupplicant);
		if (networks.isEmpty())
			qDebug("peng!");
		else
			qDebug("pong");
	}
	
	CManagedAPModel::~CManagedAPModel() {
		supplicant = NULL;
	}
	
	void CManagedAPModel::setWpaSupplicant(CWpa_Supplicant * wpaSupplicant) {
		if (supplicant == wpaSupplicant)
			return;
		
		supplicant = wpaSupplicant;
		if (supplicant) {
			updateNetworks();
			connect(supplicant, SIGNAL(networkListUpdated()), this, SLOT(updateNetworks()));
		}
	}
	
	void CManagedAPModel::updateNetworks() {
		emit layoutAboutToBeChanged();
		networks = supplicant->listNetworks();
		emit layoutChanged();
	}
	
	int CManagedAPModel::columnCount(const QModelIndex &) const {
		return 4;
	}
	
	int CManagedAPModel::rowCount(const QModelIndex & parent) const {
		if (!parent.isValid())
			return networks.count();
		else
			return 0;
	}
	
	QVariant CManagedAPModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid())
			return QVariant();
		
		if ((role == Qt::DecorationRole) && (index.column() == 0))
			return QIcon(networks[index.row()].adhoc ? UI_ICON_ADHOC : UI_ICON_AIR_ACTIVATED);
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case UI_MANAP_ID:
			return QString::number(index.row());
		case UI_MANAP_SSID:
			return networks[index.row()].ssid;
		case UI_MANAP_BSSID:
			if (networks[index.row()].bssid.zero())
				return tr("any");
			else
				return networks[index.row()].bssid.toString();
		case UI_MANAP_STATUS:
			//strange "flags"
			switch (networks[index.row()].flags) {
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
		if ((networks.isEmpty()) || (!index.isValid()))
			return 0;
		
		return (networks[index.row()].flags == NF_CURRENT) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::ItemIsSelectable;
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
		if (networks.isEmpty() || parent.isValid() || row >= networks.count())
			return QModelIndex();
		else
			return createIndex(row, column, networks[row].id);
	}
	
	QModelIndex CManagedAPModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
};
