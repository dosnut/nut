//
// C++ Implementation: deviceoptionsmodel
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
#include "availableapmodel.h"
#include "constants.h"

#define AVLAP_MOD_SSID    0
#define AVLAP_MOD_FREQ    1
#define AVLAP_MOD_KEYMGMT 2
#define AVLAP_MOD_BSSID   3
#define AVLAP_MOD_LEVEL   4
#define AVLAP_MOD_CIPHERS 5

namespace qnut {
	using namespace nut;
	
	CAvailableAPModel::CAvailableAPModel(CWpa_Supplicant * data, QObject * parent) : QAbstractItemModel(parent) {
		supplicant = data;
		if (supplicant) {
			scans = supplicant->scanResults();
			connect(supplicant, SIGNAL(opened()), this, SLOT(reloadScans()));
			connect(supplicant, SIGNAL(closed()), this, SLOT(reloadScans()));
			connect(supplicant, SIGNAL(scanCompleted()), this, SLOT(reloadScans()));
		}
	}
	
	CAvailableAPModel::~CAvailableAPModel() {
		supplicant = NULL;
	}
	
	void CAvailableAPModel::reloadScans() {
		emit layoutAboutToBeChanged();
		scans = supplicant->scanResults();
		emit layoutChanged();
	}
	
	int CAvailableAPModel::columnCount(const QModelIndex &) const {
		if (supplicant == NULL)
			return 0;
		else
			return 6;
	}
	
	int CAvailableAPModel::rowCount(const QModelIndex & parent) const {
		if (supplicant == NULL)
			return 0;
		
		if (!parent.isValid())
			return scans.count();
		else {
			return 0;
		}
	}
	
	QVariant CAvailableAPModel::data(const QModelIndex & index, int role) const {
		if (supplicant == NULL)
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case AVLAP_MOD_SSID:
			return scans[index.row()].ssid;
		case AVLAP_MOD_FREQ:
			return QString::number(scans[index.row()].freq);
		case AVLAP_MOD_KEYMGMT: {
				int flags = scans[index.row()].auth;
				if (flags == WA_PLAIN)
					return tr("none");
				
				QStringList results;
				
				if (flags & WA_WPA_PSK)
					results << tr("WPA PSK");
				if (flags & WA_WPA2_PSK)
					results << tr("WPA2 PSK");
				if (flags & WA_WPA_EAP)
					results << tr("WPA EAP");
				if (flags & WA_WPA2_EAP)
					results << tr("WPA2 EAP");
				if (flags & WA_IEEE8021X)
					results << tr("IEEE8021X");
				
				return results.join(", ");
			}
		case AVLAP_MOD_BSSID:
			return scans[index.row()].bssid.toString();
		case AVLAP_MOD_LEVEL:
			return QString::number(scans[index.row()].level);
		case AVLAP_MOD_CIPHERS: {
				int flags = scans[index.row()].ciphers;
				if (flags == WC_UNDEFINED)
					return tr("undefined");
				else if (flags == WC_NONE)
					return tr("none");
				
				QStringList results;
				
				if (flags & WC_CCMP)
					results << tr("CCMP");
				if (flags & WC_TKIP)
					results << tr("TKIP");
				if (flags & WC_WEP104)
					results << tr("WEP 104");
				if (flags & WC_WEP40)
					results << tr("WEP 40");
				if (flags & WC_WEP)
					results << tr("WEP");
				
				return results.join(", ");
			}
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CAvailableAPModel::flags(const QModelIndex & index) const {
		if (supplicant == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CAvailableAPModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (supplicant == NULL)
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case AVLAP_MOD_SSID:
				return tr("ssid");
			case AVLAP_MOD_FREQ:
				return tr("frequency");
			case AVLAP_MOD_KEYMGMT:
				return tr("key management");
			case AVLAP_MOD_BSSID:
				return tr("bssid");
			case AVLAP_MOD_LEVEL:
				return tr("level");
			case AVLAP_MOD_CIPHERS:
				return tr("ciphers");
			default:
				break;
			}
		}
		return QVariant();
	}
	
	QModelIndex CAvailableAPModel::index(int row, int column, const QModelIndex & parent) const {
		if (supplicant == NULL)
			return QModelIndex();
		
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		
		
		if (!parent.isValid()) {
			if (row < scans.count())
				return createIndex(row, column, (void *)(&(scans[row])));
		}
		
		return QModelIndex();
	}
	
	QModelIndex CAvailableAPModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
};
