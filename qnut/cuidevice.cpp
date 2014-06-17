//
// C++ Implementation: UI Device
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "cuidevice.h"

#include <QHeaderView>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QSignalMapper>
#include <libnutclient/client.h>
#include <libnutwireless/cwireless.h>

#include "common.h"
#include "constants.h"
#include "modelview/cenvironmenttreemodel.h"
#include "modelview/cenvironmentdetailsmodel.h"
#include "modelview/cinterfacedetailsmodel.h"
#include "cipconfiguration.h"
#include "cdevicesettings.h"

#ifndef QNUT_NO_WIRELESS
#include "cwirelesssettings.h"
#endif

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;

	QSignalMapper * CUIDevice::m_ShowRequestMapper = NULL;

	void CUIDevice::init() {
		m_ShowRequestMapper = new QSignalMapper(NULL);
	}

	void CUIDevice::cleanup() {
		if (m_ShowRequestMapper)
			delete m_ShowRequestMapper;
	}

	CUIDevice::CUIDevice(CDevice * parentDevice, QWidget * parent) : QWidget(parent) {
		m_Device = parentDevice;

#ifndef QNUT_NO_WIRELESS
		if (m_Device->getWireless()) {
			m_WirelessSettings = new CWirelessSettings(m_Device);
			connect(m_Device->getWireless()->getHardware(), SIGNAL(signalQualityUpdated(libnutwireless::SignalQuality)),
				this, SIGNAL(wirelessInformationUpdated()));
		}
		else
			m_WirelessSettings = NULL;
#endif

		createView();
		createActions();

		readSettings();

		connect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)),
			this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));

		if (m_Device->getState() == DeviceState::UP)
			ui.environmentTree->expand(ui.environmentTree->model()->index(m_Device->getEnvironments().indexOf(m_Device->getActiveEnvironment()), 0));
	}

	CUIDevice::~CUIDevice() {
		writeSettings();
#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings)
			delete m_WirelessSettings;
#endif

		delete m_DeviceMenu;
	}

	inline void CUIDevice::readCommands(QSettings * settings) {
		settings->beginGroup(UI_SETTINGS_COMMANDS);
		m_CommandsEnabled = settings->value(UI_SETTINGS_ENABLED, false).toBool();

		ToggleableCommand newCommand;

		for (int i = 0; i < 5; i++) {
			int size = settings->beginReadArray(toString((DeviceState)i));
			for (int k = 0; k < size; ++k) {
				settings->setArrayIndex(k);
				newCommand.path = settings->value(UI_SETTINGS_COMMAND).toString();
				newCommand.enabled = settings->value(UI_SETTINGS_ENABLED).toBool();
				m_CommandList[i] << newCommand;

			}
			settings->endArray();
		}

		settings->endGroup();
	}

	inline void CUIDevice::readIPConfigs(QSettings * settings) {
		settings->beginGroup(UI_SETTINGS_IPCONFIGURATIONS);

		foreach (CEnvironment * i, m_Device->getEnvironments()) {
			if (settings->childGroups().contains(i->getName())) {
				settings->beginGroup(i->getName());
				foreach (CInterface * j, i->getInterfaces()) {
					if (j->getConfig().flags & IPv4ConfigFlag::USERSTATIC && settings->childGroups().contains(QString::number(j->getIndex()))) {
						libnutcommon::IPv4UserConfig config;

						settings->beginGroup(QString::number(j->getIndex()));
						config.ip = QHostAddress(settings->value(UI_SETTINGS_IP).toString());
						config.netmask = QHostAddress(settings->value(UI_SETTINGS_NETMASK).toString());
						config.gateway = QHostAddress(settings->value(UI_SETTINGS_GATEWAY).toString());

						QList<QHostAddress> dnsServers;
						int size = settings->beginReadArray(UI_SETTINGS_DNSSERVERS);
						for (int k = 0; k < size; ++k) {
							settings->setArrayIndex(k);
							QHostAddress dnsServer(settings->value(UI_SETTINGS_ADDRESS).toString());
							if (!dnsServer.isNull())
								dnsServers << dnsServer;
						}
						settings->endArray();

						config.dnsServers = dnsServers;
						settings->endGroup();

						if (config.valid())
							j->setUserConfig(config);

						m_IPConfigsToRemember.insert(j);
					}
				}
				settings->endGroup();
			}
		}

		settings->endGroup();
	}

	inline void CUIDevice::readSettings() {
		QSettings * settings = new QSettings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);
#ifndef QNUT_SETTINGS_NOCOMPAT
		QString configFile = UI_PATH_DEV(m_Device->getName()) + "dev.conf";
		bool readOld = QFile::exists(UI_PATH_DEV(m_Device->getName()) + "dev.conf") && settings->childGroups().isEmpty();

		if (readOld) {
			qDebug("[QNUT] no device config found, trying legacy");
			delete settings;
			settings = new QSettings(configFile, QSettings::IniFormat);
			settings->beginGroup(UI_SETTINGS_MAIN);
		}
		else
			settings->beginGroup(m_Device->getName());
#else
		settings->beginGroup(m_Device->getName());
#endif
		m_ShowTrayIcon = settings->value(UI_SETTINGS_SHOWTRAYICON, false).toBool();
		ui.detailsButton->setChecked(settings->value(UI_SETTINGS_SHOWDETAILS, false).toBool());
		m_NotificationsEnabled = !settings->value(UI_SETTINGS_HIDENOTIFICATIONS, false).toBool();

#ifndef QNUT_SETTINGS_NOCOMPAT
		if (readOld) {
			quint8 scriptFlags = settings->value(UI_SETIINGS_SCRIPTFLAGS, 0).toInt();
			m_CommandsEnabled = scriptFlags;

			QString prefix = UI_PATH_DEV(m_Device->getName());
			ToggleableCommand newCommand;

			newCommand.path = prefix + UI_DIR_SCRIPT_UP;
			newCommand.enabled = scriptFlags & UI_FLAG_SCRIPT_UP;
			m_CommandList[(quint32) DeviceState::UP] << newCommand;

			newCommand.path = prefix + UI_DIR_SCRIPT_UNCONFIGURED;
			newCommand.enabled = scriptFlags & UI_FLAG_SCRIPT_UNCONFIGURED;
			m_CommandList[(quint32) DeviceState::UNCONFIGURED] << newCommand;

			newCommand.path = prefix + UI_DIR_SCRIPT_CARRIER;
			newCommand.enabled = scriptFlags & UI_FLAG_SCRIPT_CARRIER;
			m_CommandList[(quint32) DeviceState::CARRIER] << newCommand;

			newCommand.path = prefix + UI_DIR_SCRIPT_ACTIVATED;
			newCommand.enabled = scriptFlags & UI_FLAG_SCRIPT_ACTIVATED;
			m_CommandList[(quint32) DeviceState::ACTIVATED] << newCommand;

			newCommand.path = prefix + UI_DIR_SCRIPT_DEACTIVATED;
			newCommand.enabled = scriptFlags & UI_FLAG_SCRIPT_DEACTIVATED;
			m_CommandList[(quint32) DeviceState::DEACTIVATED] << newCommand;

			settings->endGroup();
		}
		else
			readCommands(settings);
#else
		readCommands(settings);
#endif

#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings)
			m_WirelessSettings->readSettings(settings);
#endif

		if (settings->childGroups().contains(UI_SETTINGS_IPCONFIGURATIONS))
			readIPConfigs(settings);

#ifndef QNUT_SETTINGS_NOCOMPAT
		if (!readOld)
			settings->endGroup();
#else
		settings->endGroup();
#endif

		delete settings;
	}

	inline void CUIDevice::writeCommands(QSettings * settings) {
		if (settings->childGroups().contains(UI_SETTINGS_COMMANDS))
			settings->remove(UI_SETTINGS_COMMANDS);

		settings->beginGroup(UI_SETTINGS_COMMANDS);
		settings->setValue(UI_SETTINGS_ENABLED, m_CommandsEnabled);
		for (int i = 0; i < 5; i++) {
			if (!m_CommandList[i].isEmpty()) {
				settings->beginWriteArray(toString((DeviceState)i), m_CommandList[i].size());
				for (int k = 0; k < m_CommandList[i].size(); ++k) {
					settings->setArrayIndex(k);
					settings->setValue(UI_SETTINGS_COMMAND, m_CommandList[i][k].path);
					settings->setValue(UI_SETTINGS_ENABLED, m_CommandList[i][k].enabled);
				}
				settings->endArray();
			}
		}
		settings->endGroup();
	}

	inline void CUIDevice::writeIPConfigs(QSettings * settings) {
		if (settings->childGroups().contains(UI_SETTINGS_IPCONFIGURATIONS))
			settings->remove(UI_SETTINGS_IPCONFIGURATIONS);

		if (!m_IPConfigsToRemember.isEmpty()) {
			settings->beginGroup(UI_SETTINGS_IPCONFIGURATIONS);

			foreach (CInterface * i, m_IPConfigsToRemember) {
				settings->beginGroup(qobject_cast<CEnvironment *>(i->parent())->getName());
				settings->beginGroup(QString::number(i->getIndex()));

				settings->setValue(UI_SETTINGS_IP, i->getUserConfig().ip.toString());
				settings->setValue(UI_SETTINGS_NETMASK, i->getUserConfig().netmask.toString());
				settings->setValue(UI_SETTINGS_GATEWAY, i->getUserConfig().gateway.toString());
				settings->beginWriteArray(UI_SETTINGS_DNSSERVERS, i->getUserConfig().dnsServers.size());
				for (int k = 0; k < i->getUserConfig().dnsServers.size(); ++k) {
					settings->setArrayIndex(k);
					settings->setValue(UI_SETTINGS_ADDRESS, i->getUserConfig().dnsServers[k].toString());
				}
				settings->endArray();

				settings->endGroup();
				settings->endGroup();
			}

			settings->endGroup();
		}
	}

	inline void CUIDevice::writeSettings() {
		QSettings settings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);

		settings.beginGroup(m_Device->getName());
		settings.setValue(UI_SETTINGS_SHOWTRAYICON, m_ShowTrayIcon);
		settings.setValue(UI_SETTINGS_SHOWDETAILS, ui.detailsButton->isChecked());
		settings.setValue(UI_SETTINGS_HIDENOTIFICATIONS, !m_NotificationsEnabled);

#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings)
			m_WirelessSettings->writeSettings(&settings);
#endif

		writeIPConfigs(&settings);

		settings.endGroup();
	}

	inline void CUIDevice::createActions() {
		m_DeviceMenu = new QMenu(m_Device->getName() + " - " + toStringTr(m_Device->getType()), NULL);
		m_DeviceMenu->setIcon(QIcon(iconFile(m_Device, false)));

		QAction * tempAction;

		tempAction = m_DeviceMenu->addAction(QIcon(UI_ICON_ENABLE), tr("&Enable device"),
			m_Device, SLOT(enable()));
		tempAction->setEnabled(m_Device->getState() == DeviceState::DEACTIVATED);

		tempAction = m_DeviceMenu->addAction(QIcon(UI_ICON_DISABLE), tr("&Disable device"),
			m_Device, SLOT(disable()));
		tempAction->setDisabled(m_Device->getState() == DeviceState::DEACTIVATED);

		m_DeviceMenu->addSeparator();
		m_DeviceMenu->addAction(QIcon(UI_ICON_CONFIGURE), tr("&Device settings..."),
			this, SLOT(openDeviceSettings()));
#ifndef QNUT_NO_WIRELESS
		tempAction = m_DeviceMenu->addAction(QIcon(UI_ICON_AP), tr("&Wireless settings..."),
			this, SLOT(openWirelessSettings()));
		tempAction->setEnabled(m_Device->getWireless());
#endif

		m_DeviceActions = m_DeviceMenu->actions();

		m_DeviceMenu->addSeparator();
		m_ShowEnvironmentsAction = m_DeviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("En&vironments..."),
			m_ShowRequestMapper, SLOT(map()));
		m_ShowRequestMapper->setMapping(m_ShowEnvironmentsAction, this);

		m_EnterEnvironmentAction = new QAction(QIcon(UI_ICON_FORCE), tr("E&nter environment"), this);
		m_EnterEnvironmentAction->setEnabled(false);
		ui.environmentTree->addAction(m_EnterEnvironmentAction);

		m_IPConfigurationAction  = new QAction(QIcon(UI_ICON_EDIT), tr("Set &IP configuration..."), this);
		m_IPConfigurationAction->setEnabled(false);
		ui.environmentTree->addAction(m_IPConfigurationAction);

		ui.environmentTree->setContextMenuPolicy(Qt::ActionsContextMenu);

		connect(m_IPConfigurationAction, SIGNAL(triggered()), this, SLOT(openIPConfiguration()));

		tempAction = new QAction(/*QIcon(UI_ICON_COPY),*/  tr("&Copy property"), this);
		connect(tempAction, SIGNAL(triggered()), this, SLOT(copySelectedProperty()));
		ui.detailsView->addAction(tempAction);
	}

	inline void CUIDevice::createView() {
		ui.setupUi(this);

		ui.detailsView->setContextMenuPolicy(Qt::ActionsContextMenu);

		ui.environmentTree->setModel(new CEnvironmentTreeModel(m_Device));
		ui.environmentTree->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.environmentTree->setIconSize(QSize(20,20));

		connect(ui.environmentTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &, const QItemSelection &)));
		setHeadInfo();
	}

	inline void CUIDevice::setHeadInfo() {
		ui.iconLabel->setPixmap(QPixmap(iconFile(m_Device)));

		ui.statusLabel->setText(toStringTr(m_Device->getState()));
		ui.networkLabel->setText(tr("connected to: %1").arg(currentNetwork(m_Device, false)));

		ui.networkLabel->setVisible(m_Device->getState() > DeviceState::CARRIER);
	}

	inline void CUIDevice::executeCommand(QStringList & env, QString path) {
		QProcess * process = new QProcess(this);
		process->setEnvironment(env + QProcess::systemEnvironment());
		qDebug("[QNUT] starting process: %s", path.toAscii().data());
		process->start(path);
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
		connect(process, SIGNAL(error(QProcess::ProcessError)), process, SLOT(deleteLater()));
	}

	void CUIDevice::handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
		QModelIndexList deselectedIndexes = deselected.indexes();
		QModelIndexList selectedIndexes = selected.indexes();

		if (!deselectedIndexes.isEmpty()) {
			QModelIndex targetIndex = deselectedIndexes[0];
			CEnvironment * environment;

			if (targetIndex.parent().isValid())
				environment = static_cast<CEnvironment *>(targetIndex.parent().internalPointer());
			else
				environment = static_cast<CEnvironment *>(targetIndex.internalPointer());

			disconnect(environment, SIGNAL(activeChanged(bool)), m_EnterEnvironmentAction, SLOT(setDisabled(bool)));
			disconnect(m_EnterEnvironmentAction, SIGNAL(triggered()), environment, SLOT(enter()));
		}

		QItemSelectionModel * oldSelectionModel = ui.detailsView->selectionModel();
		QAbstractItemModel * oldItemModel = ui.detailsView->model();

		if (selectedIndexes.isEmpty()) {
			m_IPConfigurationAction->setEnabled(false);
			m_EnterEnvironmentAction->setEnabled(false);
			ui.detailsView->setEnabled(false);
		}
		else {
			QModelIndex targetIndex = selectedIndexes[0];
			CEnvironment * environment;

			if (targetIndex.parent().isValid()) {
				CInterface * interface = static_cast<CInterface *>(targetIndex.internalPointer());
				environment = qobject_cast<CEnvironment *>(interface->parent());

				m_IPConfigurationAction->setEnabled(interface->getConfig().flags & libnutcommon::IPv4ConfigFlag::USERSTATIC);
				ui.detailsView->setRootIsDecorated(false);
				ui.detailsView->setModel(new CInterfaceDetailsModel(interface));
			}
			else {
				environment = static_cast<CEnvironment *>(targetIndex.internalPointer());

				m_IPConfigurationAction->setEnabled(false);
				ui.detailsView->setRootIsDecorated(true);
				ui.detailsView->setModel(new CEnvironmentDetailsModel(environment));
				ui.detailsView->expandAll();
			}

			m_EnterEnvironmentAction->setDisabled(environment->isActive());

			connect(environment, SIGNAL(activeChanged(bool)), m_EnterEnvironmentAction, SLOT(setDisabled(bool)));
			connect(m_EnterEnvironmentAction, SIGNAL(triggered()), environment, SLOT(enter()));

			ui.detailsView->setEnabled(true);

			delete oldSelectionModel;
			delete oldItemModel;
		}
	}

	void CUIDevice::openIPConfiguration() {
		CIPConfiguration dialog(this);
		dialog.setWindowIcon(QIcon(UI_ICON_EDIT));
		QModelIndex selectedIndex = (ui.environmentTree->selectionModel()->selection().indexes())[0];

		CInterface * interface = static_cast<CInterface *>(selectedIndex.internalPointer());
		libnutcommon::IPv4UserConfig config = interface->getUserConfig();
		bool remember = m_IPConfigsToRemember.contains(interface);
		if (dialog.execute(config, remember)) {
			interface->setUserConfig(config);

			if (remember)
				m_IPConfigsToRemember.insert(interface);
			else
				m_IPConfigsToRemember.remove(interface);
		}
	}

	void CUIDevice::copySelectedProperty() {
		QModelIndex target = ui.detailsView->selectionModel()->selectedRows(1)[0];
		QString property = target.data().toString();

		if (property.isEmpty()) {
			target = ui.detailsView->selectionModel()->selectedRows(0)[0];
			property = target.data().toString();
		}

		if (property.isEmpty())
			return;

		QApplication::clipboard()->setText(property);
	}

	void CUIDevice::openDeviceSettings() {
		CDeviceSettings dialog(this);
		dialog.setWindowIcon(QIcon(UI_ICON_CONFIGURE));
		dialog.setWindowTitle(tr("Device settings for %1").arg(m_Device->getName()));
		if (dialog.execute(m_CommandList, m_CommandsEnabled, m_ShowTrayIcon, m_NotificationsEnabled, true)) {
			m_ShowTrayIcon = dialog.trayIconVisibleResult();
			emit showTrayIconRequested(m_ShowTrayIcon);
			for (int i = 0; i < 5; i++)
				m_CommandList[i] = dialog.commandListsResult()[i];

			m_CommandsEnabled = dialog.commandsEnabledResult();
			m_NotificationsEnabled = dialog.notificationEnabledResult();

			QSettings settings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);

			settings.beginGroup(m_Device->getName());
			writeCommands(&settings);
			settings.endGroup();
		}
	}

#ifndef QNUT_NO_WIRELESS
	void CUIDevice::openWirelessSettings() {
		m_WirelessSettings->show();
		m_WirelessSettings->activateWindow();
	}
#endif

	void CUIDevice::handleDeviceStateChange(DeviceState state) {
		setHeadInfo();
		ui.environmentTree->collapseAll();
		if (state >= DeviceState::UNCONFIGURED)
			ui.environmentTree->expand(ui.environmentTree->model()->index(m_Device->getActiveEnvironment()->getIndex(), 0));

		m_DeviceActions[0]->setEnabled(state == DeviceState::DEACTIVATED);
		m_DeviceActions[1]->setDisabled(state == DeviceState::DEACTIVATED);

		emit updateTrayIconRequested(state);

		if (m_NotificationsEnabled)
			emit showNotificationRequested(state);

		if (m_CommandsEnabled && !(m_CommandList[(quint32) state].isEmpty())) {
			QFileInfo currentFile;
			QStringList env;

			env << "QNUT_DEV_NAME="  + m_Device->getName();
			env << "QNUT_DEV_STATE=" + libnutcommon::toString(state);

			if (state >= DeviceState::UNCONFIGURED)
				env << "QNUT_ENV_NAME=" + m_Device->getActiveEnvironment()->getName();

			if (state == DeviceState::UP) {
				env << "QNUT_IF_COUNT=" + QString::number(m_Device->getActiveEnvironment()->getInterfaces().count());
				int j = 0;
				foreach (CInterface * i, m_Device->getActiveEnvironment()->getInterfaces()) {
					env << QString("QNUT_IF_%1=%2").arg(QString::number(j),i->getIp().toString());
					j++;
				}
			}

			foreach (ToggleableCommand i, m_CommandList[(quint32) state]) {
				if (i.enabled /*TODO check if path is valid: && QFile::exists(i.path)*/) {
					currentFile.setFile(i.path);
					if (currentFile.isDir()) {
						QDir workdir(i.path);
						foreach(QString j, workdir.entryList(QDir::Executable | QDir::Files))
							executeCommand(env, workdir.filePath(j));
					}
					else
						executeCommand(env, i.path);
				}
			}
		}
	}
}

