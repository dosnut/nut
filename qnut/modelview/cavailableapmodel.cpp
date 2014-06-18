//
// C++ Implementation: deviceoptionsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef NUT_NO_WIRELESS
#include <QIcon>
#include <libnutwireless/cwireless.h>
#include <libnutwireless/conversion.h>

#include "modelview/cavailableapmodel.h"
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

	int CAvailableAPModel::scanResultIdByModelIndex(const QModelIndex & index) const {
		if (index.internalId() == -1)
			return m_GroupedScans[m_SSIDs[index.row()]]->at(0);
		else
			return index.internalId();
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

		foreach (IndexList * i, m_GroupedScans)
			delete i;

		m_GroupedScans.clear();

		IndexList * target;
		for (int i = 0; i < m_Scans.count(); i++) {
			target = m_GroupedScans.value(m_Scans[i].ssid, NULL);
			if (!target) {
				target = new IndexList();
				m_GroupedScans.insert(m_Scans[i].ssid, target);
			}

			target->append(i);
		}

		m_SSIDs = m_GroupedScans.keys();

		emit layoutChanged();
		emit cachedScansUpdated();
	}

	bool CAvailableAPModel::hasChildren(const QModelIndex & parent) const {
		return !parent.isValid() || parent.internalId() == -1;
	}

	int CAvailableAPModel::rowCount(const QModelIndex & parent) const {
		if (m_SSIDs.isEmpty())
			return 0;

		if (parent.isValid() && parent.internalId() == -1) {
			IndexList * scans = m_GroupedScans[m_SSIDs[parent.row()]];
			return scans->size();
		}

		if (!parent.isValid())
			return m_SSIDs.size();

		return 0;
	}

	int CAvailableAPModel::columnCount(const QModelIndex &) const {
		return 6;
	}

	QVariant CAvailableAPModel::data(const QModelIndex & index, int role) const {
		if (m_SSIDs.isEmpty())
			return QVariant();

		if (!index.isValid())
			return QVariant();

		int scanId = scanResultIdByModelIndex(index);

		if ((role == Qt::DecorationRole) && (index.column() == 0))
			return (m_Scans[scanId].opmode == OPM_ADHOC) ? QIcon(UI_ICON_ADHOC) : QIcon(UI_ICON_AP);

		if (role != Qt::DisplayRole)
			return QVariant();

		if (index.internalId() == -1) {
			if (index.column() == UI_AVLAP_BSSID)
				return tr("multiple");

			if (index.column() == UI_AVLAP_CHANNEL)
				return QString('-');
		}

		switch (index.column()) {
		case UI_AVLAP_SSID:
			{
				QString result = index.parent().isValid() ? '#' + QString::number(index.row() + 1) : m_Scans[scanId].ssid;
				if (m_Scans[scanId].opmode == OPM_ADHOC)
					return result + " <" + tr("ad-hoc") + '>';
				else
					return result;
			}
		case UI_AVLAP_CHANNEL:
			return QString::number(frequencyToChannel(m_Scans[scanId].freq));
		case UI_AVLAP_KEYMGMT:
			{
				int keyFlags = m_Scans[scanId].keyManagement;

				if (keyFlags == KM_UNDEFINED)
					return tr("undefined");

				if (keyFlags == KM_NONE)
					return tr("none/WEP");

				if (keyFlags == KM_WPA_NONE)
					return tr("WPA PSK (ad-hoc)");

				int protocolFlags = m_Scans[scanId].protocols;

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
			return m_Scans[scanId].bssid.toString();
		case UI_AVLAP_QUALITY:
			{
				SignalQuality signal = m_Scans[scanId].signal;
				return QString::number(signal.quality.value) + '/'+
					QString::number(signal.quality.maximum);
			}
		case UI_AVLAP_LEVEL:
			{
				SignalQuality signal = m_Scans[scanId].signal;
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
		case UI_AVLAP_ENC:
			{
				int flags = m_Scans[scanId].pairwise;
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

				switch (m_Scans[scanId].group) {
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
		if (m_SSIDs.isEmpty())
			return QModelIndex();

		if (parent.isValid() && parent.internalId() == -1)
			return createIndex(row, column, m_GroupedScans[m_SSIDs[parent.row()]]->at(row));

		if (!parent.isValid()) {
			IndexList * scans = m_GroupedScans[m_SSIDs[row]];
			return createIndex(row, column, scans->size() > 1 ? -1 : scans->at(0));
		}

		return QModelIndex();
	}

	QModelIndex CAvailableAPModel::parent(const QModelIndex & index) const {
		if (m_Scans.isEmpty() || !index.isValid())
			return QModelIndex();

		if (index.internalId() == -1)
			return QModelIndex();

		IndexList * scans = m_GroupedScans[m_Scans[index.internalId()].ssid];
		if (scans->size() > 1)
			return createIndex(m_SSIDs.indexOf(m_Scans[index.internalId()].ssid), 0, -1);

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

		int leftIndex  = source->scanResultIdByModelIndex(left);
		int rightIndex = source->scanResultIdByModelIndex(right);

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
