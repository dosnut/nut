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
#include "common.h"

#define AVLAP_MOD_SSID    0
#define AVLAP_MOD_FREQ    1
#define AVLAP_MOD_KEYMGMT 2
#define AVLAP_MOD_BSSID   4
#define AVLAP_MOD_SIGNAL  5
#define AVLAP_MOD_CIPHERS 3

namespace qnut {
	using namespace nut;
	using namespace libnutws;
	
	CAvailableAPModel::CAvailableAPModel(CWpa_Supplicant * wpaSupplicant, QObject * parent) : QAbstractItemModel(parent) {
		setWpaSupplicant(wpaSupplicant);
	}
	
	CAvailableAPModel::~CAvailableAPModel() {
		supplicant = NULL;
	}
	
	void CAvailableAPModel::setWpaSupplicant(CWpa_Supplicant * wpaSupplicant) {
		supplicant = wpaSupplicant;
		if (supplicant) {
			reloadScans();
			connect(supplicant, SIGNAL(opened()), this, SLOT(reloadScans()));
			connect(supplicant, SIGNAL(closed()), this, SLOT(reloadScans()));
			connect(supplicant, SIGNAL(scanCompleted()), this, SLOT(reloadScans()));
		}
	}
	
	void CAvailableAPModel::reloadScans() {
		emit layoutAboutToBeChanged();
		scans = supplicant->scanResults();
		emit layoutChanged();
	}
	
	int CAvailableAPModel::columnCount(const QModelIndex &) const {
		return 6;
	}
	
	int CAvailableAPModel::rowCount(const QModelIndex & parent) const {
		if (!parent.isValid())
			return scans.count();
		else
			return 0;
	}
	
	QVariant CAvailableAPModel::data(const QModelIndex & index, int role) const {
		if (scans.isEmpty())
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
				int keyFlags = scans[index.row()].keyManagement;
				
				if (keyFlags == KM_UNDEFINED)
					return tr("undefined");
				
				int protocolFlags = scans[index.row()].protocols;
				
				QStringList results;
				QStringList wpaPrefixes;
				
				if (keyFlags & KM_NONE)
					results << tr("plain or WEP");
					
				if (protocolFlags & PROTO_WPA)
					wpaPrefixes << "WPA";
				
				if (protocolFlags & PROTO_RSN)
					wpaPrefixes << "WPA2";
				
				if (keyFlags & KM_WPA_PSK) {
					results << wpaPrefixes.join("/") + " PSK";
				}
				if (keyFlags & KM_WPA_EAP) {
					results << wpaPrefixes.join("/") + " EAP";
				}
				if (keyFlags & KM_IEEE8021X) {
					results << "IEEE 802.1X";
				}
				
				return results.join(", ");
			}
		case AVLAP_MOD_BSSID:
			return scans[index.row()].bssid.toString();
		case AVLAP_MOD_SIGNAL:
			return signalSummary(scans[index.row()].signal);
		case AVLAP_MOD_CIPHERS: {
				int flags = scans[index.row()].ciphers;
				if (flags == CI_UNDEFINED)
					return tr("undefined");
				else if (flags == CI_NONE)
					return tr("none");
				
				QStringList results;
				
				if (flags & CI_CCMP)
					results << "CCMP";
				if (flags & CI_TKIP)
					results << "TKIP";
				if ((flags & CI_WEP104) || (flags & CI_WEP40))
					results << "WEP";
				
				return results.join(", ");
			}
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CAvailableAPModel::flags(const QModelIndex & index) const {
		if ((scans.isEmpty()) || (!index.isValid()))
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CAvailableAPModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case AVLAP_MOD_SSID:
				return tr("SSID");
			case AVLAP_MOD_FREQ:
				return tr("Frequency");
			case AVLAP_MOD_KEYMGMT:
				return tr("Key management");
			case AVLAP_MOD_BSSID:
				return tr("BSSID");
			case AVLAP_MOD_SIGNAL:
				return tr("Signal (Quality, Level, Noise)");
			case AVLAP_MOD_CIPHERS:
				return tr("Encryption");
			default:
				break;
			}
		}
		return QVariant();
	}
	
	QModelIndex CAvailableAPModel::index(int row, int column, const QModelIndex & parent) const {
		if (scans.isEmpty())
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
