//
// C++ Implementation: ipconfiguration
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QMessageBox>
#include "ipconfiguration.h"
#include "dnslistmodel.h"
#include "ipeditdelegate.h"

namespace qnut {
	using namespace libnutclient;
	
	CIPConfiguration::CIPConfiguration(QWidget * parent) : QDialog(parent) {
		ui.setupUi(this);
		connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addDNS()));
		connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeDNS()));
	}
	
	bool CIPConfiguration::execute(libnutcommon::IPv4UserConfig & config) {
		ui.ipEdit->setText(config.ip().toString());
		ui.netmaskEdit->setText(config.netmask().toString());
		ui.gatewayEdit->setText(config.gateway().toString());
		
		m_DNSList = config.dnsservers();
		
		ui.dnsList->setModel(new CDNSListModel(&m_DNSList));
		ui.dnsList->setItemDelegate(new CIPEditDelegate());
		
		connect(ui.dnsList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &)));
		
		if (exec()) {
			config.setIP(QHostAddress(ui.ipEdit->text()));
			config.setNetmask(QHostAddress(ui.netmaskEdit->text()));
			config.setGateway(QHostAddress(ui.gatewayEdit->text()));
			config.setDnsservers(m_DNSList);
			return true;
		}
		else
			return false;
	}
	
	void CIPConfiguration::addDNS() {
		QHostAddress address = QHostAddress(ui.dnsEdit->text());
		if (address.isNull())
			QMessageBox::information(this, tr("Faild to add dns server"), tr("Address is invalid."));
		else
			dynamic_cast<CDNSListModel *>(ui.dnsList->model())->appendRow(address);
	}
	
	void CIPConfiguration::removeDNS() {
		QModelIndexList selectedIndexes = ui.dnsList->selectionModel()->selectedIndexes();
		ui.dnsList->model()->removeRow(selectedIndexes[0].row());
	}
	
	void CIPConfiguration::handleSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		ui.removeButton->setDisabled(selectedIndexes.isEmpty());
	}
};
