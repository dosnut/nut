//
// C++ Implementation: managedapmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
/*
        TRANSLATOR qnut::QObject
*/
#ifndef NUT_NO_WIRELESS
#include <QIcon>
#include <QFont>
#include <QApplication>

#include "modelview/cmanagedapmodel.h"
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
			connect(m_Supplicant, &CWpaSupplicant::networkListUpdated, this, &CManagedAPModel::updateNetworks);
			connect(m_Supplicant, &CWpaSupplicant::stateChanged, this, &CManagedAPModel::updateNetworks);
		}
	}

	void CManagedAPModel::updateNetworks() {
		emit layoutAboutToBeChanged();
		m_Networks = m_Supplicant->listNetworks();
		m_CurrentID = -1;
		foreach (libnutwireless::ShortNetworkInfo i, m_Networks)
			if (i.flags == NF_CURRENT)
				m_CurrentID = i.id;
		emit layoutChanged();
	}

	bool CManagedAPModel::hasChildren(const QModelIndex & parent) const {
		return !parent.isValid();
	}

	int CManagedAPModel::rowCount(const QModelIndex & parent) const {
		return parent.isValid() ? 0 : m_Networks.count();
	}

	int CManagedAPModel::columnCount(const QModelIndex &) const {
		return 4;
	}

	inline QString toStringTr(NetworkFlags flags) {
		switch (flags) {
		case NF_CURRENT:
			return QObject::tr("selected");
		case NF_DISABLED:
			return QObject::tr("disabled");
		default:
			return QObject::tr("enabled");
		}
	}

	inline QString toStringTr(libnutcommon::MacAddress bssid) {
		if (bssid.zero())
			return QObject::tr("any");
		else
			return bssid.toString();
	}

	QVariant CManagedAPModel::data(const QModelIndex & index, int role) const {
		if (!index.isValid())
			return QVariant();

		if (role == Qt::ToolTipRole) {
			return tr("id: %1\nstatus: %2\nBSSID: %3").arg(index.row()).arg(
				toStringTr(m_Networks[index.row()].flags),
				toStringTr(m_Networks[index.row()].bssid)
				);
		}

		if ((role == Qt::DecorationRole) && (index.column() == 0)) {
			switch (m_Networks[index.row()].flags) {
			case NF_CURRENT:
				return QIcon(m_Networks[index.row()].adhoc ? UI_ICON_ADHOC_ACTIVATED : UI_ICON_AP_ACTIVATED);
			case NF_DISABLED:
				return QIcon(m_Networks[index.row()].adhoc ? UI_ICON_ADHOC_DOWN : UI_ICON_AP_DOWN);
			default:
				return QIcon(m_Networks[index.row()].adhoc ? UI_ICON_ADHOC : UI_ICON_AP);
			}
		}

		if (role == Qt::FontRole && m_Networks[index.row()].flags == NF_CURRENT) {
			QFont font = QApplication::font();
			font.setBold(true);
			return font;
		}

		if (role != Qt::DisplayRole)
			return QVariant();

		switch (index.column()) {
		case UI_MANAP_ID:
			return QString::number(index.row());
		case UI_MANAP_SSID:
			return m_Networks[index.row()].ssid;
		case UI_MANAP_BSSID:
			return toStringTr(m_Networks[index.row()].bssid);
		case UI_MANAP_STATUS:
			return toStringTr(m_Networks[index.row()].flags);
		default:
			break;
		}

		return QVariant();
	}

	Qt::ItemFlags CManagedAPModel::flags(const QModelIndex & index) const {
		if ((m_Networks.isEmpty()) || (!index.isValid()))
			return 0;

		return /*(m_Networks[index.row()].flags == NF_CURRENT) ? */Qt::ItemIsEnabled | Qt::ItemIsSelectable/* : Qt::ItemIsSelectable*/;
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

	QModelIndex CManagedAPModel::index(int row, int column, const QModelIndex & /*parent*/) const {
		if (m_Networks.isEmpty() || row >= m_Networks.count())
			return QModelIndex();
		else
			return createIndex(row, column, row);
	}

	QModelIndex CManagedAPModel::parent(const QModelIndex &) const {
		return QModelIndex();
	}

	CManagedAPProxyModel::CManagedAPProxyModel(QObject * parent) : QSortFilterProxyModel(parent) {
		setDynamicSortFilter(true);
	}

	bool CManagedAPProxyModel::lessThan(const QModelIndex & left, const QModelIndex & right) {
		CManagedAPModel * source = qobject_cast<CManagedAPModel *>(sourceModel());
		if (!source)
			return true;

		int leftID  = left.internalId();
		int rightID = right.internalId();

		switch (left.column()) {
			case UI_MANAP_ID:
				return lessThanID(source->cachedNetworks()[leftID], source->cachedNetworks()[rightID]);
			case UI_MANAP_SSID:
				return lessThanSSID(source->cachedNetworks()[leftID], source->cachedNetworks()[rightID]);
			case UI_MANAP_BSSID:
				return lessThanBSSID(source->cachedNetworks()[leftID], source->cachedNetworks()[rightID]);
			case UI_MANAP_STATUS:
				return lessThanFlags(source->cachedNetworks()[leftID], source->cachedNetworks()[rightID]);
		default:
			return true;
		}
	}
}
#endif
