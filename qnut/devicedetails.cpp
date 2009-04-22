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
	
	CDeviceDetails::CDeviceDetails(CDevice * parentDevice, QWidget * parent) :
		QWidget(parent),
		m_Settings(UI_PATH_DEV(parentDevice->getName()) + "dev.conf", QSettings::IniFormat, this)
	{
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
		disconnect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)),
			this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
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
		m_Settings.beginGroup("Main");
		m_ScriptFlags = m_Settings.value("scriptFlags", 0).toInt();
		m_trayIcon->setVisible(m_Settings.value("showTrayIcon", false).toBool());
		ui.detailsButton->setChecked(m_Settings.value("showDetails", false).toBool());
		m_Settings.endGroup();
		ui.showTrayCheck->setChecked(m_trayIcon->isVisible());
		
		#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings) {
			m_Settings.beginGroup("WirelessSettings");
			m_WirelessSettings->resize(m_Settings.value("size", QSize(646, 322)).toSize());
			m_WirelessSettings->move(m_Settings.value("pos", QPoint(200, 200)).toPoint());
			m_WirelessSettings->setDetailsVisible(m_Settings.value("showDetails", false).toBool());
			m_Settings.endGroup();
		}
		#endif
		
		if (m_Settings.childGroups().contains("IPconfigurations")) {
			m_Settings.beginGroup("IPconfigurations");
			
			foreach (CEnvironment * i, m_Device->getEnvironments()) {
				if (m_Settings.childGroups().contains(i->getName())) {
					m_Settings.beginGroup(i->getName());
					foreach (CInterface * j, i->getInterfaces()) {
						if (j->getConfig().getFlags() & IPv4Config::DO_USERSTATIC && m_Settings.childGroups().contains(QString::number(j->getIndex()))) {
							libnutcommon::IPv4UserConfig config;
							
							m_Settings.beginGroup(QString::number(j->getIndex()));
							config.setIP(QHostAddress(m_Settings.value("ip").toString()));
							config.setNetmask(QHostAddress(m_Settings.value("netmask").toString()));
							config.setGateway(QHostAddress(m_Settings.value("gateway").toString()));
							
							QList<QHostAddress> dnsServers;
							int size = m_Settings.beginReadArray("dnsServers");
							for (int k = 0; k < size; ++k) {
								m_Settings.setArrayIndex(k);
								QHostAddress dnsServer(m_Settings.value("address").toString());
								if (!dnsServer.isNull())
									dnsServers << dnsServer;
							}
							m_Settings.endArray();
							
							config.setDnsservers(dnsServers);
							m_Settings.endGroup();
							
							if (config.valid())
								j->setUserConfig(config);
							
							m_IPConfigsToRemember.insert(j);
						}
					}
					m_Settings.endGroup();
				}
			}
			
			m_Settings.endGroup();
		}
	}
	
	inline void CDeviceDetails::writeSettings() {
		m_Settings.beginGroup("Main");
		m_Settings.setValue("scriptFlags", m_ScriptFlags);
		m_Settings.setValue("showTrayIcon", m_trayIcon->isVisible());
		m_Settings.setValue("showDetails", ui.detailsButton->isChecked());
		m_Settings.endGroup();
		
		#ifndef QNUT_NO_WIRELESS
		if (m_WirelessSettings) {
			m_Settings.beginGroup("WirelessSettings");
			m_Settings.setValue("size", m_WirelessSettings->size());
			m_Settings.setValue("pos", m_WirelessSettings->pos());
			m_Settings.setValue("showDetails", m_WirelessSettings->detailsVisible());
			m_Settings.endGroup();
		}
		#endif
		
		if (m_Settings.childGroups().contains("IPconfigurations"))
			m_Settings.remove("IPconfigurations");
		
		if (!m_IPConfigsToRemember.isEmpty()) {
			m_Settings.beginGroup("IPconfigurations");
			
			foreach (CInterface * i, m_IPConfigsToRemember) {
				m_Settings.beginGroup(qobject_cast<CEnvironment *>(i->parent())->getName());
				m_Settings.beginGroup(QString::number(i->getIndex()));
				
				m_Settings.setValue("ip", i->getUserConfig().ip().toString());
				m_Settings.setValue("netmask", i->getUserConfig().netmask().toString());
				m_Settings.setValue("gateway", i->getUserConfig().gateway().toString());
				m_Settings.beginWriteArray("dnsServers", i->getUserConfig().dnsservers().size());
				for (int k = 0; k < i->getUserConfig().dnsservers().size(); ++k) {
					m_Settings.setArrayIndex(k);
					m_Settings.setValue("address", i->getUserConfig().dnsservers()[k].toString());
				}
				m_Settings.endArray();
				
				m_Settings.endGroup();
				m_Settings.endGroup();
			}
			
			m_Settings.endGroup();
		}
	}
	
	inline void CDeviceDetails::createActions() {
		m_DeviceMenu = new QMenu(m_Device->getName(), NULL);
		
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
	}
	
	inline void CDeviceDetails::createView() {
		m_trayIcon = new QSystemTrayIcon(QIcon(iconFile(m_Device)), this);
		
		ui.setupUi(this);
		
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
