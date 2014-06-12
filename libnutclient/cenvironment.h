#ifndef LIBNUTCLIENT_CENVIRONMENT_H
#define LIBNUTCLIENT_CENVIRONMENT_H

#include <QObject>
#include <QList>

#include "clibnut.h"

namespace libnutclient {
	class CLog;
	class DBusEnvironmentInterface;
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;
	typedef QList<CInterface *> CInterfaceList;
}

namespace libnutclient {

	/** @brief The Environment manages the interfaces

		The Environment manages its associated interfaces. It also provides information about the Environment (name, select results, state).
		It reports state changes via 3 signals

	*/
	class CEnvironment : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CInterface;
		friend class DBusEnvironmentInterface;
	private:
		//CDevice * parent;
		QDBusObjectPath m_dbusPath;
		CLog * log;
		QHash<QDBusObjectPath, CInterface *> m_dbusInterfaces;
		DBusEnvironmentInterface * m_dbusEnvironment;
		libnutcommon::EnvironmentConfig m_config;
		libnutcommon::SelectResult m_selectResult;
		QVector<libnutcommon::SelectResult> m_selectResults;

		QString m_name;
		CInterfaceList m_interfaces;
		bool m_state;
		int m_index;

		bool m_propertiesFetched;
		bool m_interfacesFetched;
		bool m_configFetched;
		bool m_selectResultFetched;
		bool m_selectResultsFetched;
		bool m_initCompleted;

		void refreshAll();
		void rebuild(const QList<QDBusObjectPath> &paths);
		void checkInitCompleted();
	private slots:
		void dbusStateChanged(bool state);
		void dbusSelectResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);

		//dbus return functions
		void dbusretGetInterfaces(QList<QDBusObjectPath>);
		void dbusretGetProperties(libnutcommon::EnvironmentProperties properties);
		void dbusretGetConfig(libnutcommon::EnvironmentConfig config);
		void dbusretGetSelectResult(libnutcommon::SelectResult selectResult);
		void dbusretGetSelectResults(QVector<libnutcommon::SelectResult> selectResults);

		void dbusret_errorOccured(QDBusError error, QString method = QString());

		//init slots for interface:
		void interfaceInitializationFailed(CInterface * interface);
		void interfaceInitializationCompleted(CInterface * interface);

	public:

		void init();

		inline const QString& getName() { return m_name; }
		inline const CInterfaceList& getInterfaces() { return m_interfaces;}
		inline bool getState() const { return m_state; }
		inline int getIndex() const { return m_index;}

		CEnvironment(CDevice * parent, QDBusObjectPath dbusPath);
		~CEnvironment();
	public slots:
		void enter();
		libnutcommon::EnvironmentConfig& getConfig();
		libnutcommon::SelectResult& getSelectResult(bool refresh=false);
		QVector<libnutcommon::SelectResult>& getSelectResults(bool refresh=false);


	signals:
		void initializationFailed(CEnvironment * environment); //TODO:Implement this: has to be called if init fails
		void initializationCompleted(CEnvironment * environment);

		void newDataAvailable();

		/** @brief Environment signals

			activeChanged(bool active) is emitted when environment get's activated or deactivated
			interfacesUpdated() is emitted when interfaces are added or removed
			selectResultsChanged() is emitted when select results changed (this normaly hapens, when they're completly done)

		*/
		void activeChanged(bool active);
		void interfacesUpdated();
		void selectResultsChanged();
	};
}

#endif
