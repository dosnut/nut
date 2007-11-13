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

#define MANAP_MOD_ID    0
#define MANAP_MOD_SSID  1
#define MANAP_MOD_FLAG  2
#define MANAP_MOD_BSSID 3

namespace qnut {
	using namespace nut;
	using namespace libnutws;
	
	CManagedAPModel::CManagedAPModel(CWpa_Supplicant * wpaSupplicant, QObject * parent) : QAbstractItemModel(parent) {
		setWpaSupplicant(wpaSupplicant);
	}
	
	CManagedAPModel::~CManagedAPModel() {
		supplicant = NULL;
	}
	
	void CManagedAPModel::setWpaSupplicant(CWpa_Supplicant * wpaSupplicant) {
		supplicant = wpaSupplicant;
		if (supplicant) {
			reloadNetworks();
			connect(supplicant, SIGNAL(opened()), this, SLOT(reloadNetworks()));
			connect(supplicant, SIGNAL(closed()), this, SLOT(reloadNetworks()));
			connect(supplicant, SIGNAL(stateChanged(bool)), this, SLOT(reloadNetworks()));
		}
	}
	
	void CManagedAPModel::reloadNetworks() {
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
		if ((networks.isEmpty()) || (!index.isValid()))
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case MANAP_MOD_ID:
			return QString::number(index.row());
		case MANAP_MOD_SSID:
			return networks[index.row()].ssid;
		case MANAP_MOD_BSSID:
			if (networks[index.row()].bssid.zero())
				return tr("any");
			else
				return networks[index.row()].bssid.toString();
		case MANAP_MOD_FLAG:
			return (networks[index.row()].flags == NF_CURRENT) ? QString('*') : QVariant();
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CManagedAPModel::flags(const QModelIndex & index) const {
		if ((networks.isEmpty()) || (!index.isValid()))
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CManagedAPModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (networks.isEmpty())
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case MANAP_MOD_ID:
				return tr("ID");
			case MANAP_MOD_SSID:
				return tr("SSID");
			case MANAP_MOD_BSSID:
				return tr("BSSID");
			case MANAP_MOD_FLAG:
				return tr("current");
			default:
				break;
			}
		}
		return QVariant();
	}
	
	QModelIndex CManagedAPModel::index(int row, int column, const QModelIndex & parent) const {
		if (networks.isEmpty())
			return QModelIndex();
		
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		
		if (!parent.isValid()) {
			if (row < networks.count())
				return createIndex(row, column, (void *)(&(networks[row])));
		}
		
		return QModelIndex();
	}
	
	QModelIndex CManagedAPModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
};
