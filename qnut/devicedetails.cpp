//
// C++ Implementation: deviceoptions
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QHeaderView>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include "devicedetails.h"
#include "common.h"
#include "environmenttreemodel.h"
#include "interfacedetailsmodel.h"
#include "environmentdetailsmodel.h"
#include "ipconfiguration.h"
#include "scriptsettings.h"
#include "wirelesssettings.h"

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;
	
	CDeviceDetails::CDeviceDetails(CDevice * parentDevice, QWidget * parent) :
		QWidget(parent),
		m_Stettings(UI_PATH_DEV(parentDevice->name) + "dev.conf", QSettings::IniFormat, this)
	{
		m_Device = parentDevice;
		
		if (m_Device->type == DT_AIR)
			m_WirelessSettings = new CWirelessSettings(m_Device);
		else
			m_WirelessSettings = NULL;
		
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
		
		if (m_Device->state == DS_UP)
			ui.environmentTree->expand(ui.environmentTree->model()->index(m_Device->environments.indexOf(m_Device->activeEnvironment), 0));
	}
	
	CDeviceDetails::~CDeviceDetails() {
		disconnect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)),
			this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
		writeSettings();
		if (m_WirelessSettings) {
			m_WirelessSettings->close();
			delete m_WirelessSettings;
		}
		delete m_DeviceMenu;
	}
	
	inline void CDeviceDetails::readSettings() {
		m_Stettings.beginGroup("Main");
		m_ScriptFlags = m_Stettings.value("m_ScriptFlags", 0).toInt();
		m_trayIcon->setVisible(m_Stettings.value("showTrayIcon", false).toBool());
		ui.detailsButton->setChecked(m_Stettings.value("showDetails", false).toBool());
		m_Stettings.endGroup();
		ui.showTrayCheck->setChecked(m_trayIcon->isVisible());
		
		if (m_WirelessSettings) {
			m_Stettings.beginGroup("WirelessSettings");
			m_WirelessSettings->resize(m_Stettings.value("size", QSize(646, 322)).toSize());
			m_WirelessSettings->move(m_Stettings.value("pos", QPoint(200, 200)).toPoint());
			m_WirelessSettings->setDetailsVisible(m_Stettings.value("showDetails", false).toBool());
			m_Stettings.endGroup();
		}
	}
	
	inline void CDeviceDetails::writeSettings() {
		m_Stettings.beginGroup("Main");
		m_Stettings.setValue("scriptFlags", m_ScriptFlags);
		m_Stettings.setValue("showTrayIcon", m_trayIcon->isVisible());
		m_Stettings.setValue("showDetails", ui.detailsButton->isChecked());
		m_Stettings.endGroup();
		
		if (m_WirelessSettings) {
			m_Stettings.beginGroup("WirelessSettings");
			m_Stettings.setValue("size", m_WirelessSettings->size());
			m_Stettings.setValue("pos", m_WirelessSettings->pos());
			m_Stettings.setValue("showDetails", m_WirelessSettings->detailsVisible());
			m_Stettings.endGroup();
		}
	}
	
	inline void CDeviceDetails::createActions() {
		m_DeviceMenu = new QMenu(m_Device->name, NULL);
		
		m_DeviceMenu->addAction(QIcon(UI_ICON_ENABLE), tr("&Enable device"),
			m_Device, SLOT(enable()));
		m_DeviceMenu->addAction(QIcon(UI_ICON_DISABLE), tr("&Disable device"),
			m_Device, SLOT(disable()));
		m_DeviceMenu->addSeparator();
		m_DeviceMenu->addAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("&Scripting settings..."),
			this, SLOT(openDeviceSettings()));
		m_DeviceMenu->addAction(QIcon(UI_ICON_AIR), tr("&Wireless settings..."),
			this, SLOT(openWirelessSettings()));
		
		m_DeviceActions = m_DeviceMenu->actions();
		
		m_DeviceMenu->addSeparator();
		m_DeviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("En&vironments..."),
			this, SLOT(showTheeseDetails()));
		
		m_EnterEnvironmentAction = new QAction(QIcon(UI_ICON_FORCE), tr("E&nter environment"), this);
		m_IPConfigurationAction  = new QAction(QIcon(UI_ICON_EDIT), tr("Set &IP configuration..."), this);
		ui.environmentTree->addAction(m_EnterEnvironmentAction);
		ui.environmentTree->addAction(m_IPConfigurationAction);
		
		ui.environmentTree->setContextMenuPolicy(Qt::ActionsContextMenu);

		m_DeviceActions[0]->setEnabled(m_Device->state == DS_DEACTIVATED);
		m_DeviceActions[1]->setDisabled(m_Device->state == DS_DEACTIVATED);
		m_DeviceActions[4]->setEnabled(m_Device->type == DT_AIR);
		m_IPConfigurationAction->setEnabled(false);
		m_EnterEnvironmentAction->setEnabled(false);
		
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
		ui.statusLabel->setText(toStringTr(m_Device->state));
	}
	
	void CDeviceDetails::handleTrayActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			showTheeseDetails();
		}
	}
	
	void CDeviceDetails::showTheeseDetails() {
		emit showOptionsRequested(this);
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
				environment = dynamic_cast<CEnvironment *>(interface->parent());
				
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
			
			m_EnterEnvironmentAction->setDisabled(environment->active);
			
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
		if (dialog.execute(config))
			interface->setUserConfig(config);
	}
	
	void CDeviceDetails::openDeviceSettings() {
		CScriptSettings dialog(this);
		dialog.execute(this);
	}
	
	void CDeviceDetails::openWirelessSettings() {
		m_WirelessSettings->show();
		m_WirelessSettings->activateWindow();
	}
	
	void CDeviceDetails::handleDeviceStateChange(DeviceState state) {
		setHeadInfo();
		ui.environmentTree->collapseAll();
		if (state >= DS_UNCONFIGURED)
			ui.environmentTree->expand(ui.environmentTree->model()->index(m_Device->activeEnvironment->index, 0));
		
		m_DeviceActions[0]->setEnabled(state == DS_DEACTIVATED);
		m_DeviceActions[1]->setDisabled(state == DS_DEACTIVATED);
		
		if (m_trayIcon->isVisible()) {
			m_trayIcon->setToolTip(shortSummary(m_Device));
			m_trayIcon->setIcon(QIcon(iconFile(m_Device)));
			
			switch (state) {
			case DS_UP:
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->name),
					tr("... is now up and running."), m_trayIcon);
				break;
			case DS_UNCONFIGURED:
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->name),
					tr("... got carrier but needs configuration.\n\nKlick here to open the device details."), m_trayIcon);
				break;
			case DS_ACTIVATED:
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->name),
					tr("... is now activated an waits for carrier."), m_trayIcon);
				break;
			case DS_DEACTIVATED: 
				emit showMessageRequested(tr("QNUT - %1 ...").arg(m_Device->name),
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
					tr("%1 is now up and running.").arg(m_Device->name));
				break;
			case DS_UNCONFIGURED:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 got carrier but needs configuration.\n\nKlick here to open the device details.").arg(m_Device->name));
				break;
			case DS_ACTIVATED:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 is now activated an waits for carrier.").arg(m_Device->name));
				break;
			case DS_DEACTIVATED:
				emit showMessageRequested(tr("QNUT"),
					tr("%1 is now deactivated").arg(m_Device->name));
				break;
			default:
				break;
			}
		}
		if (m_ScriptFlags) {
			QDir workdir(UI_PATH_DEV(m_Device->name));
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
				QProcess process;
				env << "QNUT_DEV_NAME="  + m_Device->name;
				env << "QNUT_DEV_STATE=" + libnutcommon::toString(state);
				
				if (state >= DS_UNCONFIGURED)
					env << "QNUT_ENV_NAME=" + m_Device->activeEnvironment->name;
				
				if (state == DS_UP) {
					env << "QNUT_IF_COUNT=" + QString::number(m_Device->activeEnvironment->interfaces.count());
					int j = 0;
					foreach (CInterface * i, m_Device->activeEnvironment->interfaces) {
						env << QString("QNUT_IF_%1=%2").arg(QString::number(j),i->ip.toString());
						j++;
					}
				}
				
				process.setEnvironment(env);
				workdir.cd(targetDir);
				foreach(QString i, workdir.entryList()) {
					process.startDetached(workdir.path() + '/' + i);
				}
			}
		}
	}
};
