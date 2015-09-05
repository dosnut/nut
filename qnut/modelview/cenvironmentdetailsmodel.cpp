//
// C++ Implementation: environmentdetailsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QIcon>
#include <libnutclient/cdevice.h>
#include <libnutclient/cenvironment.h>

#include "modelview/cenvironmentdetailsmodel.h"
#include "constants.h"

#define ENVDET_MOD_STATEMENT 0
#define ENVDET_MOD_VALUE     1

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;

	void CEnvironmentDetailsModel::fillParentRules(quint32 start) {
		if ((m_SelectConfig.filters[start].selType == SelectType::AND_BLOCK) ||
		    (m_SelectConfig.filters[start].selType == SelectType::OR_BLOCK)) {
			quint32 block = m_SelectConfig.filters[start].block;
			foreach (quint32 i, m_SelectConfig.blocks[block]) {
				m_ParentRules[i] = start;
				fillParentRules(i);
			}
		}
	}

	CEnvironmentDetailsModel::CEnvironmentDetailsModel(CEnvironment * data, QObject * parent) : QAbstractItemModel(parent) {
		m_Environment = data;
		if (m_Environment) {
			m_SelectConfig = data->getConfig().select;

			m_ParentRules.resize(m_SelectConfig.filters.size());
			m_ParentRules[0] = 0;
			fillParentRules();

			connect(m_Environment, SIGNAL(activeChanged(bool)), this, SIGNAL(layoutChanged()));
		}
	}

	CEnvironmentDetailsModel::~CEnvironmentDetailsModel() {
		m_Environment = NULL;
	}

	int CEnvironmentDetailsModel::columnCount(const QModelIndex &) const {
		if (m_Environment == NULL)
			return 0;
		else
			return 2;
	}

	int CEnvironmentDetailsModel::rowCount(const QModelIndex & parent) const {
		if (m_Environment == NULL)
			return 0;

		if (parent.column() > 0)
			return 0;

		if (!parent.isValid())
			return 1;

		if ((m_SelectConfig.filters[parent.internalId()].selType == SelectType::AND_BLOCK) ||
			(m_SelectConfig.filters[parent.internalId()].selType == SelectType::OR_BLOCK))
			return m_SelectConfig.blocks[m_SelectConfig.filters[parent.internalId()].block].size();

		return 0;
	}

	QVariant CEnvironmentDetailsModel::data(const QModelIndex & index, int role) const {
		if ((m_Environment == NULL) || (!index.isValid()))
			return QVariant();

		if ((role == Qt::DecorationRole) && (index.column() == ENVDET_MOD_STATEMENT)) {
			CDevice * device = qobject_cast<CDevice *>(m_Environment->parent());
			if (m_Environment == device->getActiveEnvironment()) {
				if ((qint8)(m_Environment->getSelectResults()[index.internalId()]))
					return QIcon(UI_ICON_SELECTED);
			}
			return QIcon(UI_ICON_UNSELECTED);
		}

		if (role != Qt::DisplayRole)
			return QVariant();

		switch (index.column()) {
			case ENVDET_MOD_STATEMENT:
				if (m_SelectConfig.filters[index.internalId()].invert)
					switch (m_SelectConfig.filters[index.internalId()].selType) {
					case SelectType::USER:
						return tr("not by user");
					case SelectType::ARP:
						return tr("not by arp");
					case SelectType::ESSID:
						return tr("not by ssid");
					case SelectType::AND_BLOCK:
						return tr("at least one not");
					case SelectType::OR_BLOCK:
						return tr("all not");
					default:
						break;
					}
				else
					switch (m_SelectConfig.filters[index.internalId()].selType) {
					case SelectType::USER:
						return tr("by user");
					case SelectType::ARP:
						return tr("by arp");
					case SelectType::ESSID:
						return tr("by ssid");
					case SelectType::AND_BLOCK:
						return tr("all");
					case SelectType::OR_BLOCK:
						return tr("at least one");
					default:
						break;
					}
			case ENVDET_MOD_VALUE:
				switch (m_SelectConfig.filters[index.internalId()].selType) {
				case SelectType::ARP:
					if (m_SelectConfig.filters[index.internalId()].macAddr.valid())
						return m_SelectConfig.filters[index.internalId()].ipAddr.toString() + ", "+
							m_SelectConfig.filters[index.internalId()].macAddr.toString();
					else
						return m_SelectConfig.filters[index.internalId()].ipAddr.toString();
				case SelectType::ESSID:
					return m_SelectConfig.filters[index.internalId()].essid;
				default:
					break;
				}
			default:
				break;
		}

		return QVariant();
	}

	Qt::ItemFlags CEnvironmentDetailsModel::flags(const QModelIndex & index) const {
		if (m_Environment == NULL)
			return 0;

		if (!index.isValid())
			return 0;

		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant CEnvironmentDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (m_Environment == NULL)
			return QVariant();

		if (role != Qt::DisplayRole)
			return QVariant();

		if (orientation == Qt::Horizontal) {
			switch (section) {
			case ENVDET_MOD_STATEMENT:
				return tr("Statement");
			case ENVDET_MOD_VALUE:
				return tr("Value");
			default:
				break;
			}
		}
		return QVariant();
	}

	QModelIndex CEnvironmentDetailsModel::index(int row, int column, const QModelIndex & parent) const {
		if (m_Environment == NULL)
			return QModelIndex();

		if (!hasIndex(row, column, parent))
			return QModelIndex();

		if (!parent.isValid()) {
			return createIndex(row, column, nullptr);
		}
		else {
			quint32 parentBlock = m_SelectConfig.filters[parent.internalId()].block;
			return createIndex(row, column, m_SelectConfig.blocks[parentBlock][row]);
		}
	}

	QModelIndex CEnvironmentDetailsModel::parent(const QModelIndex & index) const {
		if (m_Environment == NULL)
			return QModelIndex();

		if (!index.isValid())
			return QModelIndex();

		if (index.internalId() == 0)
			return QModelIndex();

		quint32 parentRule = m_ParentRules[index.internalId()];
		quint32 parentRuleIndex = 0;

		if (parentRule != 0) {
			quint32 parentBlock = m_SelectConfig.filters[m_ParentRules[parentRule]].block;
			parentRuleIndex = m_SelectConfig.blocks[parentBlock].indexOf(parentRule);
		}

		return createIndex(parentRuleIndex, 0, parentRule);
	}
}
