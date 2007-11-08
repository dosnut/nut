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
	using namespace libnut;
	
	CManagedAPModel::CManagedAPModel(CWpa_Supplicant * data, QObject * parent) : QAbstractItemModel(parent) {
		supplicant = data;
		if (supplicant) {
			networks = supplicant->listNetworks();
			connect(supplicant, SIGNAL(opened()), this, SLOT(reloadNetworks()));
			connect(supplicant, SIGNAL(closed()), this, SLOT(reloadNetworks()));
			connect(supplicant, SIGNAL(stateChanged(bool)), this, SLOT(reloadNetworks()));
		}
	}
	
	CManagedAPModel::~CManagedAPModel() {
		supplicant = NULL;
	}
	
	void CManagedAPModel::reloadNetworks() {
		emit layoutAboutToBeChanged();
		networks = supplicant->listNetworks();
		emit layoutChanged();
	}
	
	int CManagedAPModel::columnCount(const QModelIndex &) const {
		if (supplicant == NULL)
			return 0;
		else
			return 4;
	}
	
	int CManagedAPModel::rowCount(const QModelIndex & parent) const {
		if (supplicant == NULL)
			return 0;
		
		if (!parent.isValid())
			return networks.count();
		else {
			return 0;
		}
	}
	
	QVariant CManagedAPModel::data(const QModelIndex & index, int role) const {
		if (supplicant == NULL)
			return QVariant();
		
		if (!index.isValid())
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
			return (networks[index.row()].flags == WNF_CURRENT) ? QString('*') : QVariant();
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CManagedAPModel::flags(const QModelIndex & index) const {
		if (supplicant == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CManagedAPModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (supplicant == NULL)
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
		if (supplicant == NULL)
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
