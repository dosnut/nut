//
// C++ Implementation: ipconfiguration
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ipconfiguration.h"
#include "dnslistmodel.h"
#include "ipeditdelegate.h"
#include <QDebug>
namespace qnut {
	CIPConfiguration::CIPConfiguration(QWidget * parent) : QDialog(parent) {
		ui.setupUi(this);
	}
	
	CIPConfiguration::~CIPConfiguration() {
	}
	
	bool CIPConfiguration::execute(nut::IPv4UserConfig & config) {
		ui.ipEdit->setText(config.ip().toString());
		ui.netmaskEdit->setText(config.netmask().toString());
		ui.gatewayEdit->setText(config.gateway().toString());
		
		QList<QHostAddress> dnsList;
		
		ui.dnsList->setModel(new CDNSListModel(&dnsList));
		ui.dnsList->setItemDelegate(new CIPEditDelegate());
		
		if (exec()) {
			config.setIP(QHostAddress(ui.ipEdit->text()));
			config.setNetmask(QHostAddress(ui.netmaskEdit->text()));
			config.setGateway(QHostAddress(ui.gatewayEdit->text()));
			config.setDnsservers(dnsList);
			return true;
		}
		else
			return false;
	}
};
