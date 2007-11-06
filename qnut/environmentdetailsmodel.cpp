//
// C++ Implementation: environmentdetailsmodel
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
#include "environmentdetailsmodel.h"
#include "constants.h"

#define ENVDET_MOD_STATEMENT 0
#define ENVDET_MOD_VALUE     1

namespace qnut {
	using namespace nut;
	
	void CEnvironmentDetailsModel::fillParentRules(quint32 start) {
		if ((selectConfig.filters[start].selType == SelectRule::SEL_AND_BLOCK) ||
		    (selectConfig.filters[start].selType == SelectRule::SEL_OR_BLOCK)) {
			quint32 block = selectConfig.filters[start].block;
			foreach (quint32 i, selectConfig.blocks[block]) {
				parentRules[i] = start;
				fillParentRules(i);
			}
		}
	}
	
	CEnvironmentDetailsModel::CEnvironmentDetailsModel(CEnvironment * data, QObject * parent) : QAbstractItemModel(parent) {
		environment = data;
		if (environment) {
			selectConfig = data->getConfig().getSelect();
			
			parentRules.resize(selectConfig.filters.size());
			parentRules[0] = 0;
			fillParentRules();
			
			connect(environment, SIGNAL(activeChanged(bool)), this, SIGNAL(layoutChanged()));
		}
	}
	
	CEnvironmentDetailsModel::~CEnvironmentDetailsModel() {
		environment = NULL;
	}
	
	int CEnvironmentDetailsModel::columnCount(const QModelIndex &) const {
		if (environment == NULL)
			return 0;
		else
			return 2;
	}
	
	int CEnvironmentDetailsModel::rowCount(const QModelIndex & parent) const {
		if (environment == NULL)
			return 0;
		
		if (parent.column() > 0)
			return 0;
		
		if (!parent.isValid())
			return 1;
		
		if ((selectConfig.filters[parent.internalId()].selType == SelectRule::SEL_AND_BLOCK) ||
			(selectConfig.filters[parent.internalId()].selType == SelectRule::SEL_OR_BLOCK))
			return selectConfig.blocks[selectConfig.filters[parent.internalId()].block].size();
		
		return 0;
	}
	
	QVariant CEnvironmentDetailsModel::data(const QModelIndex & index, int role) const {
		if (environment == NULL)
			return QVariant();
		
		if (!index.isValid())
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		switch (index.column()) {
			case ENVDET_MOD_STATEMENT:
				switch (selectConfig.filters[index.internalId()].selType) {
				case SelectRule::SEL_USER:
					return tr("user");
				case SelectRule::SEL_ARP:
					return tr("arp");
				case SelectRule::SEL_ESSID:
					return tr("ssid");
				case SelectRule::SEL_AND_BLOCK:
					return tr("AND");
				case SelectRule::SEL_OR_BLOCK:
					return tr("OR");
				default:
					break;
				}
				break;
			case ENVDET_MOD_VALUE:
				switch (selectConfig.filters[index.internalId()].selType) {
				case SelectRule::SEL_ARP:
					if (selectConfig.filters[index.internalId()].macAddr.valid())
						return selectConfig.filters[index.internalId()].ipAddr.toString() + ", "+
							selectConfig.filters[index.internalId()].macAddr.toString();
					else
						return selectConfig.filters[index.internalId()].ipAddr.toString();
				case SelectRule::SEL_ESSID:
					return selectConfig.filters[index.internalId()].essid;
				default:
					break;
				}
				break;
			default:
				break;
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags CEnvironmentDetailsModel::flags(const QModelIndex & index) const {
		if (environment == NULL)
			return 0;
		
		if (!index.isValid())
			return 0;
		
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant CEnvironmentDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (environment == NULL)
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
		if (environment == NULL)
			return QModelIndex();
		
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		
		if (!parent.isValid()) {
			return createIndex(row, column, 0);
		}
		else {
			quint32 parentBlock = selectConfig.filters[parent.internalId()].block;
			return createIndex(row, column, selectConfig.blocks[parentBlock][row]);
		}
	}
	
	QModelIndex CEnvironmentDetailsModel::parent(const QModelIndex & index) const {
		if (environment == NULL)
			return QModelIndex();
		
		if (!index.isValid())
			return QModelIndex();
		
		if (index.internalId() == 0)
			return QModelIndex();
		
		quint32 parentRule = parentRules[index.internalId()];
		quint32 parentRuleIndex = 0;
		
		if (parentRule != 0) {
			quint32 parentBlock = selectConfig.filters[parentRules[parentRule]].block;
			parentRuleIndex = selectConfig.blocks[parentBlock].indexOf(parentRule);
		}
		
		return createIndex(parentRuleIndex, 0, parentRule);
	}
};
