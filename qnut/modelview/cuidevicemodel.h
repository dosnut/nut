#ifndef QNUT_CUIDEVICEMODEL_H
#define QNUT_CUIDEVICEMODEL_H

#include <QAbstractItemModel>
#include <QList>

namespace libnutclient {
	class CDevice;
}

namespace qnut {
	class CUIDevice;

	/**
	 * @brief CUIDeviceModel provides an item model to manage the ui representation of devicess.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * The class provides all functions for a read-only model specified in the Qt 4 documentation.
	 *
	 * The model supports the display the following information in columns for each device:
	 *  - name
	 *  - current status (deactivated, activated, got carrier, unconfigured, up)
	 *  - type (wireless, ethernet)
	 *  - name of the active environment
	 *  - current IP address
	 *  - current network ("Local" for ethernet)
	 */
	class CUIDeviceModel : public QAbstractItemModel {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object and initializes the model.
		 * @param parent parent object
		 */
		CUIDeviceModel(QObject * parent = 0);
		/// @brief Destroyes the object.
		~CUIDeviceModel();

		CUIDevice * addUIDevice(libnutclient::CDevice * device);
		void removeUIDevice(CUIDevice * target);
		void removeUIDevice(int index);

		int findUIDevice(libnutclient::CDevice * device);

		const QList<CUIDevice *> & uiDevices() const { return m_UIDevices; }

		QVariant data(const QModelIndex & index, int role) const;
		Qt::ItemFlags flags(const QModelIndex & index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column , const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
	private:
		QList<CUIDevice *> m_UIDevices;
	private slots:
		void updateDeviceState();
		void updateSignalQuality();
	};
}
#endif // QNUT_CUIDEVICEMODEL_H
