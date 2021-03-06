//
// C++ Implementation: CIPConfiguration
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QSettings>

#include "cipconfiguration.h"
#include "modelview/cdnslistmodel.h"
#include "modelview/cipeditdelegate.h"

namespace qnut {
	using namespace libnutcommon;

	CIPConfiguration::CIPConfiguration(QWidget* parent) : QDialog(parent) {
		ui.setupUi(this);
		connect(ui.addButton, &QPushButton::clicked, this, &CIPConfiguration::addDNS);
		connect(ui.removeButton, &QPushButton::clicked, this, &CIPConfiguration::removeDNS);

		connect(ui.importButton, &QPushButton::clicked, this, &CIPConfiguration::importConfig);
		connect(ui.exportButton, &QPushButton::clicked, this, &CIPConfiguration::exportConfig);
	}
	CIPConfiguration::~CIPConfiguration() = default;

	bool CIPConfiguration::execute(libnutcommon::IPv4UserConfig& config, bool& remember) {
		ui.rememberCheck->setChecked(remember);
		ui.ipEdit->setText(config.ip.toString());
		ui.netmaskEdit->setText(config.netmask.toString());
		ui.gatewayEdit->setText(config.gateway.toString());

		m_dnsListModel.reset(new CDNSListModel(config.dnsServers));

		ui.dnsList->setModel(m_dnsListModel.get());
		ui.dnsList->setItemDelegate(new CIPEditDelegate());

		connect(ui.dnsList->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &CIPConfiguration::handleSelectionChanged);

		auto const result = exec();
		ui.dnsList->setModel(nullptr);

		if (result == QDialog::Accepted) {
			config.ip = QHostAddress(ui.ipEdit->text());
			config.netmask = QHostAddress(ui.netmaskEdit->text());
			config.gateway = QHostAddress(ui.gatewayEdit->text());
			config.dnsServers = m_dnsListModel->list();

			remember = ui.rememberCheck->isChecked();

			return true;
		}
		else {
			return false;
		}
	}

	void CIPConfiguration::addDNS() {
		QHostAddress address = QHostAddress(ui.dnsEdit->text());
		if (address.isNull())
			QMessageBox::information(this, tr("Adding failed"), tr("Address is invalid."));
		else
			qobject_cast<CDNSListModel *>(ui.dnsList->model())->appendRow(address);
	}

	void CIPConfiguration::removeDNS() {
		QModelIndexList selectedIndexes = ui.dnsList->selectionModel()->selectedIndexes();
		ui.dnsList->model()->removeRow(selectedIndexes[0].row());
	}

	void CIPConfiguration::handleSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		ui.removeButton->setDisabled(selectedIndexes.isEmpty());
	}

	void CIPConfiguration::importConfig() {
		QString fileName = QFileDialog::getOpenFileName(this, tr("Import IP configuration"), QDir::currentPath(), tr("Configuration files (*.ipconf *.conf)"));
		if (!fileName.isEmpty()) {
			QSettings settings(fileName, QSettings::IniFormat);
			if (!settings.childGroups().contains("IPConfiguration"))
				return;

			settings.beginGroup("IPConfiguration");

			ui.ipEdit->setText(settings.value("ip").toString());
			ui.netmaskEdit->setText(settings.value("netmask").toString());
			ui.gatewayEdit->setText(settings.value("gateway").toString());

			ui.dnsList->model()->removeRows(0, ui.dnsList->model()->rowCount());

			int size = settings.beginReadArray("dnsServers");
			for (int k = 0; k < size; ++k) {
				settings.setArrayIndex(k);
				QHostAddress dnsServer(settings.value("address").toString());
				if (!dnsServer.isNull()) m_dnsListModel->appendRow(dnsServer);
			}
			settings.endArray();

			settings.endGroup();
		}
	}

	void CIPConfiguration::exportConfig() {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Import IP configuration"), QDir::currentPath(), tr("Configuration files (*.ipconf *.conf)"));
		if (!fileName.isEmpty()) {
			QSettings settings(fileName, QSettings::IniFormat);
			settings.beginGroup("IPConfiguration");

			settings.setValue("ip", ui.ipEdit->text());
			settings.setValue("netmask", ui.netmaskEdit->text());
			settings.setValue("gateway", ui.gatewayEdit->text());
			auto dnsList = m_dnsListModel->list();
			settings.beginWriteArray("dnsServers", dnsList.size());
			for (int k = 0; k < dnsList.size(); ++k) {
				settings.setArrayIndex(k);
				settings.setValue("address", dnsList[k].toString());
			}
			settings.endArray();

			settings.endGroup();
		}
	}
}
