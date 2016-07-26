//
// C++ Interface: environmentdetailsmodel
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_ENVIRONMENTDETAILSMODEL_H
#define QNUT_ENVIRONMENTDETAILSMODEL_H

#include <QAbstractItemModel>
#include <libnutcommon/config.h>

namespace libnutclient {
	class CEnvironment;
}

namespace qnut {
	/**
	 * @brief CInterfaceDetailsModel provides an item model for an overview of the select statements from an environment.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 *
	 * The model supports the display the following information in columns for each select statement:
	 *  - statement (all (and block), at least one (or block), by user, by arp, by ssid)
	 *  - value (ARP statement, SSID)
	 */
	class CEnvironmentDetailsModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model according to the given environment.
		 * @param data pointer to an environment (if NULL nothing is displayed)
		 * @param parent parent object
		 */
		explicit CEnvironmentDetailsModel(libnutclient::CEnvironment* data = nullptr, QObject* parent = nullptr);

		QVariant data(const QModelIndex & index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex & index) const override;
		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		int columnCount(const QModelIndex & parent = QModelIndex()) const override;

	private slots:
		void layoutChangedDefault();

	private:
		void fillParentRules(quint32 start = 0);

		libnutclient::CEnvironment* const m_Environment{nullptr};
		libnutcommon::SelectConfig m_SelectConfig;
		QVector<quint32> m_ParentRules;
	};
}

#endif
