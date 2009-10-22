//
// C++ Implementation: deviceoptions
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QHeaderView>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <libnutclient/cdevice.h>
#include <libnutclient/cenvironment.h>
#include <libnutclient/cinterface.h>

#include "devicedetails.h"
#include "common.h"
#include "constants.h"
#include "environmenttreemodel.h"
#include "interfacedetailsmodel.h"
#include "environmentdetailsmodel.h"
#include "ipconfiguration.h"
#include "scriptsettings.h"

#ifndef QNUT_NO_WIRELESS
#include "wirelesssettings.h"
#endif

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;
	
	CDeviceDetails::CDeviceDetails(CDevice * parentDevice, QWidget * parent) : QWidget(parent) {
		m_Device = parentDevice;
		
#ifndef QNUT_NO_WIRELESS
		if (m_Device->getWpaSupplicant())
			m_WirelessSettings = new CWirelessSettings(m_Device);
		else
			m_WirelessSettings = NULL;
#endif
		
		createView();
		createActions();
		
		m_trayIcon->setToolTip(shortSummary(m_Device));
		m_trayIcon->setContextMenu(m_DeviceMenu);
		
		connect(m_trayIcon, SIGNAL(messageClicked()), this, SLOT(showTheeseDetails()));
		connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(handleTrayActivated(QSystemTrayIcon::ActivationReason)));
		
		readSettings();
		
		connect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)),
			this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
		
		if (m_Device->getState() == DS_UP)
			ui.environmentTree->expand(ui.environmentTree->model()->index(m_Device->getEnvironments().indexOf(m_Device->getActiveEnvironment()), 0));
	}
	
	CDeviceDetails::~CDeviceDetails() {
//		disconnect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)),
//			this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
		writeSettings();
#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings) {
			m_WirelessSettings->close();
			delete m_WirelessSettings;
		}
#endif
		delete m_DeviceMenu;
	}
	
	inline void CDeviceDetails::readSettings() {
		QSettings * settings = new QSettings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);
#ifndef QNUT_SETTINGS_NOCOMPAT
		QString configFile = UI_PATH_DEV(m_Device->getName()) + "dev.conf";
		bool readOld = QFile::exists(UI_PATH_DEV(m_Device->getName()) + "dev.conf") && settings->childGroups().isEmpty();
		
		if (readOld) {
			qDebug("will read old config");
			delete settings;
			settings = new QSettings(configFile, QSettings::IniFormat);
			settings->beginGroup(UI_SETTINGS_MAIN);
		}
		else
			settings->beginGroup(m_Device->getName());
#else
		settings->beginGroup(m_Device->getName());
#endif
		m_ScriptFlags = settings->value(UI_SETIINGS_SCRIPTFLAGS, 0).toInt();
		m_trayIcon->setVisible(settings->value(UI_SETTINGS_SHOWTRAYICON, false).toBool());
		ui.detailsButton->setChecked(settings->value(UI_SETTINGS_SHOWDETAILS, false).toBool());
		
#ifndef QNUT_SETTINGS_NOCOMPAT
		if (readOld)
			settings->endGroup();
#endif
		
		ui.showTrayCheck->setChecked(m_trayIcon->isVisible());
		
#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings) {
			settings->beginGroup(UI_SETTINGS_WIRELESSSETTINGS);
			m_WirelessSettings->restoreGeometry(settings->value(UI_SETTINGS_GEOMETRY).toByteArray());
			m_WirelessSettings->setDetailsVisible(settings->value(UI_SETTINGS_SHOWDETAILS, false).toBool());
			settings->endGroup();
		}
#endif
		
		if (settings->childGroups().contains(UI_SETTINGS_IPCONFIGURATIONS)) {
			settings->beginGroup(UI_SETTINGS_IPCONFIGURATIONS);
			
			foreach (CEnvironment * i, m_Device->getEnvironments()) {
				if (settings->childGroups().contains(i->getName())) {
					settings->beginGroup(i->getName());
					foreach (CInterface * j, i->getInterfaces()) {
						if (j->getConfig().getFlags() & IPv4Config::DO_USERSTATIC && settings->childGroups().contains(QString::number(j->getIndex()))) {
							libnutcommon::IPv4UserConfig config;
							
							settings->beginGroup(QString::number(j->getIndex()));
							config.setIP(QHostAddress(settings->value(UI_SETTINGS_IP).toString()));
							config.setNetmask(QHostAddress(settings->value(UI_SETTINGS_NETMASK).toString()));
							config.setGateway(QHostAddress(settings->value(UI_SETTINGS_GATEWAY).toString()));
							
							QList<QHostAddress> dnsServers;
							int size = settings->beginReadArray(UI_SETTINGS_DNSSERVERS);
							for (int k = 0; k < size; ++k) {
								settings->setArrayIndex(k);
								QHostAddress dnsServer(settings->value(UI_SETTINGS_ADDRESS).toString());
								if (!dnsServer.isNull())
									dnsServers << dnsServer;
							}
							settings->endArray();
							
							config.setDnsservers(dnsServers);
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
#ifndef QNUT_SETTINGS_NOCOMPAT
		if (!readOld)
			settings->endGroup();
#else
		settings->endGroup();
#endif
		
		delete settings;
	}
	
	inline void CDeviceDetails::writeSettings() {
		QSettings settings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);
		
		settings.beginGroup(m_Device->getName());
		settings.setValue(UI_SETIINGS_SCRIPTFLAGS, m_ScriptFlags);
		settings.setValue(UI_SETTINGS_SHOWTRAYICON, m_trayIcon->isVisible());
		settings.setValue(UI_SETTINGS_SHOWDETAILS, ui.detailsButton->isChecked());
		
#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings) {
			settings.beginGroup(UI_SETTINGS_WIRELESSSETTINGS);
			settings.setValue(UI_SETTINGS_GEOMETRY, m_WirelessSettings->saveGeometry());
			settings.setValue(UI_SETTINGS_SHOWDETAILS, m_WirelessSettings->detailsVisible());
			settings.endGroup();
		}
#endif
		
		if (settings.childGroups().contains(UI_SETTINGS_IPCONFIGURATIONS))
			settings.remove(UI_SETTINGS_IPCONFIGURATIONS);
		
		if (!m_IPConfigsToRemember.isEmpty()) {
			settings.beginGroup(UI_SETTINGS_IPCONFIGURATIONS);
			
			foreach (CInterface * i, m_IPConfigsToRemember) {
				settings.beginGroup(qobject_cast<CEnvironment *>(i->parent())->getName());
				settings.beginGroup(QString::number(i->getIndex()));
				
				settings.setValue(UI_SETTINGS_IP, i->getUserConfig().ip().toString());
				settings.setValue(UI_SETTINGS_NETMASK, i->getUserConfig().netmask().toString());
				settings.setValue(UI_SETTINGS_GATEWAY, i->getUserConfig().gateway().toString());
				settings.beginWriteArray(UI_SETTINGS_DNSSERVERS, i->getUserConfig().dnsservers().size());
				for (int k = 0; k < i->getUserConfig().dnsservers().size(); ++k) {
					settings.setArrayIndex(k);
					settings.setValue(UI_SETTINGS_ADDRESS, i->getUserConfig().dnsservers()[k].toString());
				}
				settings.endArray();
				
				settings.endGroup();
				settings.endGroup();
			}
			
			settings.endGroup();
		}
		
		settings.endGroup();
	}
	
	inline void CDeviceDetails::createActions() {
		m_DeviceMenu = new QMenu(m_Device->getName(), NULL);
		m_DeviceMenu->setIcon(QIcon(iconFile(m_Device, false)));
		
		QAction * tempAction;
		
		tempAction = m_DeviceMenu->addAction(QIcon(UI_ICON_ENABLE), tr("&Enable device"),
			m_Device, SLOT(enable()));
		tempAction->setEnabled(m_Device->getState() == DS_DEACTIVATED);
		
		tempAction = m_DeviceMenu->addAction(QIcon(UI_ICON_DISABLE), tr("&Disable device"),
			m_Device, SLOT(disable()));
		tempAction->setDisabled(m_Device->getState() == DS_DEACTIVATED);
		
		m_DeviceMenu->addSeparator();
		m_DeviceMenu->addAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("&Scripting settings..."),
			this, SLOT(openScriptingSettings()));
#ifndef QNUT_NO_WIRELESS
		tempAction = m_DeviceMenu->addAction(QIcon(UI_ICON_AIR), tr("&Wireless settings..."),
			this, SLOT(openWirelessSettings()));
		tempAction->setEnabled(m_Device->getWpaSupplicant());
#endif
		
		m_DeviceActions = m_DeviceMenu->actions();
		
		m_DeviceMenu->addSeparator();
		m_DeviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("En&vironments..."),
			this, SLOT(showTheeseDetails()));
		
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
	
	inline void CDeviceDetails::createView() {
		m_trayIcon = new QSystemTrayIcon(QIcon(iconFile(m_Device)), this);
		
		ui.setupUi(this);
		
		ui.detailsView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		connect(ui.showTrayCheck, SIGNAL(toggled(bool)), m_trayIcon, SLOT(setVisible(bool)));
		
		ui.environmentTree->setModel(new CEnvironmentTreeModel(m_Device));
		ui.environmentTree->header()->setResizeMode(QHeaderView::ResizeToContents);
		
		connect(ui.environmentTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &, const QItemSelection &)));
		setHeadInfo();
	}
	
	inline void CDeviceDetails::setHeadInfo() {
		ui.iconLabel->setPixmap(QPixmap(iconFile(m_Device)));
		
		ui.statusLabel->setText(toStringTr(m_Device->getState()));
		ui.networkLabel->setText(tr("connected to: %1").arg(currentNetwork(m_Device, false)));
		
		ui.networkLabel->setVisible(m_Device->getState() > DS_CARRIER);
	}
	
	void CDeviceDetails::handleTrayActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			showTheeseDetails();
		}
	}
	
	void CDeviceDetails::showTheeseDetails() {
		emit showDetailsRequested(this);
	}
	
	void CDeviceDetails::handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
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
				
				m_IPConfigurationAction->setEnabled(interface->getConfig().getFlags() & libnutcommon::IPv4Config::DO_USERSTATIC);
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
			
			m_EnterEnvironmentAction->setDisabled(environment->getState());
			
			connect(environment, SIGNAL(activeChanged(bool)), m_EnterEnvironmentAction, SLOT(setDisabled(bool)));
			connect(m_EnterEnvironmentAction, SIGNAL(triggered()), environment, SLOT(enter()));
			
			ui.detailsView->setEnabled(true);
			
			delete oldSelectionModel;
			delete oldItemModel;
		}
	}
	
	void CDeviceDetails::openIPConfiguration() {
		CIPConfiguration dialog(this);
		QModelIndex selectedIndex = (ui.environmentTree->selectionModel()->selection().indexes())[0];
		
		CInterface * interface = static_cast<CInterface *>(selectedIndex.internalPointer());
		libnutcommon::IPv4UserConfig config = interface->getUserConfig(true);
		bool remember = m_IPConfigsToRemember.contains(interface);
		if (dialog.execute(config, remember)) {
			interface->setUserConfig(config);
			
			if (remember)
				m_IPConfigsToRemember.insert(interface);
			else
				m_IPConfigsToRemember.remove(interface);
		}
	}
	
	void CDeviceDetails::copySelectedProperty() {
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
	
	void CDeviceDetails::openScriptingSettings() {
		CScriptSettings dialog(this);
		dialog.execute(this);
	}
	
#ifndef QNUT_NO_WIRELESS
	void CDeviceDetails::openWirelessSettings() {
		m_WirelessSettings->show();
		m_WirelessSettings->activateWindow();
	}
#endif
	
	void CDeviceDetails::handleDeviceStateChange(DeviceState state) {
		setHeadInfo();
		ui.environmentTree->collapseAll();
		if (state >= DS_UNCONFIGURED)
			ui.environmentTree->expand(ui.environmentTree->model()->index(m_Device->getActiveEnvironment()->getIndex(), 0));
		
		m_DeviceActions[0]->setEnabled(state == DS_DEACTIVATED);
		m_DeviceActions[1]->setDisabled(state == DS_DEACTIVATED);
		
		m_trayIcon->setToolTip(shortSummary(m_Device));
		m_trayIcon->setIcon(QIcon(iconFile(m_Device)));
		if (m_trayIcon->isVisible()) {
			switch (state) {
			case DS_UP:
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->getName()),
					tr("... is now up and running."), m_trayIcon);
				break;
			case DS_UNCONFIGURED:
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->getName()),
					tr("... got carrier but needs configuration.\n\nKlick here to open the device details."), m_trayIcon);
				break;
			case DS_ACTIVATED:
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->getName()),
					tr("... is now activated an waits for carrier."), m_trayIcon);
				break;
			case DS_DEACTIVATED: 
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->getName()),
					tr("... is now deactivated"), m_trayIcon);
				break;
			default:
				break;
			}
		}
		else {
			switch (state) {
			case DS_UP:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 is now up and running.").arg(m_Device->getName()));
				break;
			case DS_UNCONFIGURED:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 got carrier but needs configuration.\n\nKlick here to open the device details.").arg(m_Device->getName()));
				break;
			case DS_ACTIVATED:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 is now activated an waits for carrier.").arg(m_Device->getName()));
				break;
			case DS_DEACTIVATED:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 is now deactivated").arg(m_Device->getName()));
				break;
			default:
				break;
			}
		}
		if (m_ScriptFlags) {
			QDir workdir(UI_PATH_DEV(m_Device->getName()));
			bool doExecuteScripts = false;
			QString targetDir;
			switch (state) {
			case DS_UP:
				doExecuteScripts = (m_ScriptFlags & UI_FLAG_SCRIPT_UP);
				targetDir = UI_DIR_SCRIPT_UP;
				break;
			case DS_UNCONFIGURED:
				doExecuteScripts = (m_ScriptFlags & UI_FLAG_SCRIPT_UNCONFIGURED);
				targetDir = UI_DIR_SCRIPT_UNCONFIGURED;
				break;
			case DS_CARRIER:
				doExecuteScripts = (m_ScriptFlags & UI_FLAG_SCRIPT_CARRIER);
				targetDir = UI_DIR_SCRIPT_CARRIER;
				break;
			case DS_ACTIVATED:
				doExecuteScripts = (m_ScriptFlags & UI_FLAG_SCRIPT_ACTIVATED);
				targetDir = UI_DIR_SCRIPT_ACTIVATED;
				break;
			case DS_DEACTIVATED:
				doExecuteScripts = (m_ScriptFlags & UI_FLAG_SCRIPT_DEACTIVATED);
				targetDir = UI_DIR_SCRIPT_DEACTIVATED;
				break;
			default:
				break;
			}
			
			if (doExecuteScripts && workdir.exists(targetDir)) {
				QStringList env;
				env << "QNUT_DEV_NAME="  + m_Device->getName();
				env << "QNUT_DEV_STATE=" + libnutcommon::toString(state);
				
				if (state >= DS_UNCONFIGURED)
					env << "QNUT_ENV_NAME=" + m_Device->getActiveEnvironment()->getName();
				
				if (state == DS_UP) {
					env << "QNUT_IF_COUNT=" + QString::number(m_Device->getActiveEnvironment()->getInterfaces().count());
					int j = 0;
					foreach (CInterface * i, m_Device->getActiveEnvironment()->getInterfaces()) {
						env << QString("QNUT_IF_%1=%2").arg(QString::number(j),i->getIp().toString());
						j++;
					}
				}
				
				workdir.cd(targetDir);
				foreach(QString i, workdir.entryList()) {
					QProcess * process = new QProcess(this);
					process->setEnvironment(env);
					process->start(workdir.filePath(i));
					connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
					connect(process, SIGNAL(error(QProcess::ProcessError)), process, SLOT(deleteLater()));
				}
			}
		}
	}
}
