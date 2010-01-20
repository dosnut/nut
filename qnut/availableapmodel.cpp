//
// C++ Implementation: deviceoptionsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_NO_WIRELESS
#include <QIcon>
#include <libnutwireless/cwireless.h>
#include <libnutwireless/conversion.h>
#include "availableapmodel.h"
#include "common.h"
#include "constants.h"

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;
	
	CAvailableAPModel::CAvailableAPModel(CWirelessHW * WirelessAcces, QObject * parent) : QAbstractItemModel(parent) {
		setWpaSupplicant(WirelessAcces);
	}
	
	CAvailableAPModel::~CAvailableAPModel() {
		m_WirelessAcces = NULL;
	}
	
	void CAvailableAPModel::setWpaSupplicant(CWirelessHW * wpaSupplicant) {
		if (m_WirelessAcces == wpaSupplicant)
			return;
		
		m_WirelessAcces = wpaSupplicant;
		if (m_WirelessAcces) {
			updateScans();
			connect(m_WirelessAcces, SIGNAL(scanCompleted()), this, SLOT(updateScans()));
		}
	}
	
	void CAvailableAPModel::updateScans() {
		emit layoutAboutToBeChanged();
		m_Scans = m_WirelessAcces->getScanResults();
		emit layoutChanged();
	}
	
	bool CAvailableAPModel::hasChildren(const QModelIndex & parent) const {
		return !parent.isValid();
	}
	
	int CAvailableAPModel::rowCount(const QModelIndex & parent) const {
		return parent.isValid() ? 0 : m_Scans.count();
	}
	
	int CAvailableAPModel::columnCount(const QModelIndex &) const {
		return 6;
	}
	
	QVariant CAvailableAPModel::data(const QModelIndex & index, int role) const {
		if (m_Scans.isEmpty())
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
		if ((role == Qt::DecorationRole) && (index.column() == 0))
			return QIcon(UI_ICON_AIR);
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
		case UI_AVLAP_SSID:
			return m_Scans[index.row()].ssid;
		case UI_AVLAP_CHANNEL:
			return QString::number(frequencyToChannel(m_Scans[index.row()].freq));
		case UI_AVLAP_KEYMGMT: {
				int keyFlags = m_Scans[index.row()].keyManagement;
				
				if (keyFlags == KM_UNDEFINED)
					return tr("undefined");
				
				if (keyFlags == KM_OFF)
					return tr("none");
				
				if (keyFlags == KM_NONE)
					return tr("WEP");
				
				if (keyFlags == KM_WPA_NONE)
					return tr("WPA PSK (ad-hoc)");
				
				int protocolFlags = m_Scans[index.row()].protocols;
				
				QStringList results;
				QStringList wpaPrefixes;
				
				if (protocolFlags & PROTO_WPA)
					wpaPrefixes << "WPA";
				
				if (protocolFlags & PROTO_RSN)
					wpaPrefixes << "WPA2";
				
				if (keyFlags & KM_WPA_PSK)
					results << wpaPrefixes.join("/") + " PSK";
				
				if ((keyFlags & KM_WPA_EAP) || (keyFlags & KM_IEEE8021X))
					results << wpaPrefixes.join("/") + " EAP";
				
				return results.join(", ");
			}
		case UI_AVLAP_BSSID:
			return m_Scans[index.row()].bssid.toString();
		case UI_AVLAP_QUALITY: {
				SignalQuality signal = m_Scans[index.row()].signal;
				return QString::number(signal.quality.value) + '/'+
					QString::number(signal.quality.maximum);
			}
		case UI_AVLAP_LEVEL: {
				SignalQuality signal = m_Scans[index.row()].signal;
				switch (signal.type) {
				case WSR_RCPI:
					return QString::number(signal.level.rcpi) + "dBm";
				case WSR_ABSOLUTE:
					return QString::number(signal.level.nonrcpi.value) + "dBm";
				case WSR_RELATIVE:
					return QString::number(signal.level.nonrcpi.value) + '/' +
						QString::number(signal.level.nonrcpi.maximum);
				default:
					return QString('-');
				}
			}
		case UI_AVLAP_ENC: {
				int flags = m_Scans[index.row()].pairwise;
				QStringList results;
				
				if (flags == PCI_UNDEFINED)
					results << tr("undefined");
				else if (flags == PCI_NONE)
					results << tr("none");
				else {
					if (flags & PCI_CCMP)
						results << "CCMP";
					if (flags & PCI_TKIP)
						results << "TKIP";
				}
				
				switch (m_Scans[index.row()].group) {
				case GCI_NONE:
					return tr("none") + "; " + results.join(", ");
				case GCI_WEP104:
				case GCI_WEP40:
					return "WEP; " + results.join(", ");
				case GCI_TKIP:
					return "TKIP; " + results.join(", ");
				case GCI_CCMP:
					return "CCMP; " + results.join(", ");
				default:
					return tr("undefined") + "; " + results.join(", ");
				}
			}
		default:
			break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CAvailableAPModel::flags(const QModelIndex & index) const {
		if ((m_Scans.isEmpty()) || (!index.isValid()))
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CAvailableAPModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case UI_AVLAP_SSID:
				return tr("SSID");
			case UI_AVLAP_CHANNEL:
				return tr("Channel");
			case UI_AVLAP_KEYMGMT:
				return tr("Key management");
			case UI_AVLAP_BSSID:
				return tr("BSSID");
			case UI_AVLAP_QUALITY:
				return tr("Quality");
			case UI_AVLAP_LEVEL:
				return tr("Level");
			case UI_AVLAP_ENC:
				return tr("Encryption (general; pairwise)");
			default:
				break;
			}
		}
		return QVariant();
	}
	
	QModelIndex CAvailableAPModel::index(int row, int column, const QModelIndex & parent) const {
		if (!m_Scans.isEmpty() || parent.isValid() || row >= m_Scans.count())
			return createIndex(row, column, row);
		else
			return QModelIndex();
	}
	
	QModelIndex CAvailableAPModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}
	
	CAvailableAPProxyModel::CAvailableAPProxyModel(QObject * parent) : QSortFilterProxyModel(parent) {
		setDynamicSortFilter(true);
		setFilterKeyColumn(0);
		setFilterCaseSensitivity(Qt::CaseInsensitive);
	}
	
	bool CAvailableAPProxyModel::lessThan(const QModelIndex & left, const QModelIndex & right) {
		CAvailableAPModel * source = qobject_cast<CAvailableAPModel *>(sourceModel());
		if (!source)
			return true;
		
		int leftIndex  = left.internalId();
		int rightIndex = right.internalId();
		
		switch (left.column()) {
		case UI_AVLAP_SSID:
			return lessThanSSID(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		case UI_AVLAP_CHANNEL:
			return lessThanFreq(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		case UI_AVLAP_KEYMGMT:
			return lessThanKeyManagement(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		case UI_AVLAP_BSSID:
			return lessThanBSSID(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		case UI_AVLAP_QUALITY:
			return lessThanSignalQuality(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		case UI_AVLAP_LEVEL:
			return lessThanSignalLevel(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		case UI_AVLAP_ENC:
			return lessThanGroup(source->cachedScans()[leftIndex], source->cachedScans()[rightIndex]);
		default:
			return true;
		}
	}
}
#endif
