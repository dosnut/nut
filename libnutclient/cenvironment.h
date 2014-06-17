#ifndef LIBNUTCLIENT_CENVIRONMENT_H
#define LIBNUTCLIENT_CENVIRONMENT_H

#include <QObject>
#include <QList>

#include "cnutservice.h"
#include "libnutcommon/common.h"

namespace libnutclient {
	class DBusEnvironment;
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;
	typedef QList<CInterface *> CInterfaceList;
}

namespace libnutclient {

	/** @brief The Environment manages the interfaces

		The Environment manages its associated interfaces. It also provides
		information about the Environment (name, select results, active).
	*/
	class CEnvironment : public CNutServiceClient {
		Q_OBJECT
	private:
		friend class CDevice;
		friend class CInterface;

		CDevice* m_device = nullptr;
		DBusEnvironment* m_dbusEnvironment = nullptr;
		QDBusObjectPath m_dbusPath;

		int m_initCounter = 0;
		bool checkInitDone();
		void checkInitDone(bool previous);

		CInterfaceList m_interfaces;
		QHash<QDBusObjectPath, CInterface*> m_dbusInterfaces;

		libnutcommon::EnvironmentProperties m_properties;
		libnutcommon::EnvironmentConfig m_config;

		int const m_index;

		void updateLogPrefix();
		void clear();

		/* callbacks from CInterface */
		void initializationFailed(CInterface* interface);
		void initializationCompleted(CInterface* interface);

	protected:
		void dbusLostService() override;
		void dbusConnectService(QString service, QDBusConnection connection) override;

	private slots:
		void dbusPropertiesChanged(libnutcommon::EnvironmentProperties properties);

	public:
		CEnvironment(CDevice* parent, QDBusObjectPath dbusPath, int index);
		~CEnvironment();

		QDBusObjectPath path() const { return m_dbusPath; }
		/* local index - might be the same as the server "index" property */
		int getIndex() const { return m_index;}
		qint32 getID() const { return m_properties.id; }

		QString getName() const { return m_properties.name; }
		bool isActive() const { return m_properties.active; }
		const CInterfaceList& getInterfaces() const { return m_interfaces;}

		const libnutcommon::EnvironmentConfig& getConfig() const { return m_config; }
		libnutcommon::SelectResult getSelectResult() const { return m_properties.selectResult; }
		const QVector<libnutcommon::SelectResult>& getSelectResults() const { return m_properties.selectResults; }

	public slots:
		void enter();

	signals:
		void newDataAvailable();

		/** @brief Environment signals

			activeChanged(bool active) is emitted when environment get's activated or deactivated
			selectResultsChanged() is emitted when select result(s) changed (this normaly happens when they're completly done)
		*/
		void activeChanged(bool active);
		void selectResultsChanged();
	};
}

#endif
