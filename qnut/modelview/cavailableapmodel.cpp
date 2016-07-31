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
#include <libnutwireless/cwirelesshw.h>
#include <libnutwireless/conversion.h>

#include "modelview/cavailableapmodel.h"
#include "common.h"
#include "constants.h"

namespace {
	enum {
		UI_AVLAP_SSID    = 0,
		UI_AVLAP_KEYMGMT = 1,
		UI_AVLAP_ENC     = 2,
		UI_AVLAP_BSSID   = 3,
		UI_AVLAP_CHANNEL = 4,
		UI_AVLAP_QUALITY = 5,
		UI_AVLAP_LEVEL   = 6,
	};
}

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;

	CAvailableAPModel::CAvailableAPModel(CWirelessHW* wirelessAccess, QObject* parent)
	: QAbstractItemModel(parent)
	, m_WirelessAccess(wirelessAccess) {
		if (m_WirelessAccess) {
			updateScans();
			connect(m_WirelessAccess, &CWirelessHW::scanCompleted, this, &CAvailableAPModel::updateScans);
		}
	}

	int CAvailableAPModel::scanResultIdByModelIndex(const QModelIndex& index) const {
		if (index.internalId() == quintptr(-1)) {
			if (index.row() < 0 || index.row() >= m_SSIDs.size()) return -1;
			return m_GroupedScans[m_SSIDs.at(index.row())].at(0);
		} else {
			return index.internalId();
		}
	}

	libnutwireless::ScanResult const* CAvailableAPModel::scanResultByModelIndex(const QModelIndex& index) const {
		int const id = scanResultIdByModelIndex(index);
		if (id < 0 || id >= m_Scans.size()) return nullptr;
		return &m_Scans.at(id);
	}

	QList<libnutwireless::ScanResult const*> CAvailableAPModel::scanResultListBySSID(libnutcommon::SSID const& ssid) const {
		QList<libnutwireless::ScanResult const*> scanResults;
		for (auto id: m_GroupedScans.value(ssid)) {
			scanResults.append(&m_Scans[id]);
		}
		return scanResults;
	}

	void CAvailableAPModel::updateScans() {
		emit layoutAboutToBeChanged();

		m_Scans = m_WirelessAccess->getScanResults();

		m_GroupedScans.clear();

		for (int i = 0; i < m_Scans.count(); i++) {
			m_GroupedScans[m_Scans[i].ssid].append(i);
		}

		m_SSIDs = m_GroupedScans.keys();

		emit layoutChanged();
		emit cachedScansUpdated();
	}

	bool CAvailableAPModel::hasChildren(const QModelIndex & parent) const {
		return !parent.isValid() || parent.internalId() == quintptr(-1);
	}

	int CAvailableAPModel::rowCount(const QModelIndex & parent) const {
		if (m_SSIDs.isEmpty()) return 0;

		if (parent.isValid() && parent.internalId() == quintptr(-1)) {
			return m_GroupedScans.value(m_SSIDs[parent.row()]).size();
		}

		if (!parent.isValid()) return m_SSIDs.size();

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

		auto const scanResult = scanResultByModelIndex(index);
		if (!scanResult) return QVariant();

		if ((role == Qt::DecorationRole) && (index.column() == 0))
			return (scanResult->opmode == OPM_ADHOC) ? QIcon(UI_ICON_ADHOC) : QIcon(UI_ICON_AP);

		if (role != Qt::DisplayRole)
			return QVariant();

		if (index.internalId() == quintptr(-1)) {
			if (index.column() == UI_AVLAP_BSSID)
				return tr("multiple");

			if (index.column() == UI_AVLAP_CHANNEL)
				return QString('-');
		}

		switch (index.column()) {
		case UI_AVLAP_SSID:
			{
				QString result = index.parent().isValid() ? '#' + QString::number(index.row() + 1) : scanResult->ssid.autoQuoteHexString();
				if (scanResult->opmode == OPM_ADHOC)
					return result + " <" + tr("ad-hoc") + '>';
				else
					return result;
			}
		case UI_AVLAP_CHANNEL:
			return QString::number(frequencyToChannel(scanResult->freq));
		case UI_AVLAP_KEYMGMT:
			{
				int keyFlags = scanResult->keyManagement;

				if (keyFlags == KM_UNDEFINED)
					return tr("undefined");

				if (keyFlags == KM_NONE)
					return tr("none/WEP");

				if (keyFlags == KM_WPA_NONE)
					return tr("WPA PSK (ad-hoc)");

				int protocolFlags = scanResult->protocols;

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
			return scanResult->bssid.toString();
		case UI_AVLAP_QUALITY:
			{
				SignalQuality signal = scanResult->signal;
				return QString::number(signal.quality.value) + '/'+
					QString::number(signal.quality.maximum);
			}
		case UI_AVLAP_LEVEL:
			{
				SignalQuality signal = scanResult->signal;
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
				int flags = scanResult->pairwise;
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

				switch (scanResult->group) {
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

		if (parent.isValid() && parent.internalId() == quintptr(-1)) {
			if (parent.row() >= m_SSIDs.size()) return QModelIndex();
			auto const& groupIndices = m_GroupedScans[m_SSIDs.at(parent.row())];
			if (row >= groupIndices.size()) return QModelIndex();
			return createIndex(row, column, groupIndices.at(row));
		}

		if (!parent.isValid()) {
			if (row >= m_SSIDs.size()) return QModelIndex();
			auto const& groupIndices = m_GroupedScans[m_SSIDs.at(row)];
			return createIndex(row, column, groupIndices.size() > 1 ? quintptr(-1) : groupIndices.at(0));
		}

		return QModelIndex();
	}

	QModelIndex CAvailableAPModel::parent(const QModelIndex & index) const {
		if (m_Scans.isEmpty() || !index.isValid()) return QModelIndex();

		if (index.internalId() == quintptr(-1)) return QModelIndex();

		if (index.internalId() >= static_cast<unsigned int>(m_Scans.size())) return QModelIndex();

		auto const ssid = m_Scans.at(index.internalId()).ssid;
		auto const& groupIndices = m_GroupedScans[ssid];
		if (groupIndices.size() > 1) {
			return createIndex(m_SSIDs.indexOf(ssid), 0, quintptr(-1));
		}

		return QModelIndex();
	}

	CAvailableAPProxyModel::CAvailableAPProxyModel(QObject * parent) : QSortFilterProxyModel(parent) {
		setDynamicSortFilter(true);
		setFilterKeyColumn(0);
		setFilterCaseSensitivity(Qt::CaseInsensitive);
	}

	bool CAvailableAPProxyModel::lessThan(const QModelIndex & left, const QModelIndex & right) const {
		CAvailableAPModel * source = qobject_cast<CAvailableAPModel *>(sourceModel());
		if (!source)
			return true;

		auto const* leftScan = source->scanResultByModelIndex(left);
		auto const* rightScan = source->scanResultByModelIndex(right);
		if (!rightScan) return leftScan;
		if (!leftScan) return false;

		switch (left.column()) {
		case UI_AVLAP_SSID:
			return lessThanSSID(*leftScan, *rightScan);
		case UI_AVLAP_CHANNEL:
			return lessThanFreq(*leftScan, *rightScan);
		case UI_AVLAP_KEYMGMT:
			return lessThanKeyManagement(*leftScan, *rightScan);
		case UI_AVLAP_BSSID:
			return lessThanBSSID(*leftScan, *rightScan);
		case UI_AVLAP_QUALITY:
			return lessThanSignalQuality(*leftScan, *rightScan);
		case UI_AVLAP_LEVEL:
			return lessThanSignalLevel(*leftScan, *rightScan);
		case UI_AVLAP_ENC:
			return lessThanGroup(*leftScan, *rightScan);
		default:
			return true;
		}
	}
}
#endif
