/*
    QNUT - Qt client for the Network UTility server
    Copyright (C) 2010  Oliver Groß <z.o.gross@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#ifndef QNUT_NO_WIRELESS
#include "cabstractwifinetconfigdialog.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QRegExpValidator>

#include <libnutwireless/wpa_supplicant.h>
#include <libnutwireless/wstypes.h>
#include <libnutwireless/cwireless.h>

#include "cerrorcodeevaluator.h"

namespace qnut {
	using namespace libnutwireless;
	
	QRegExpValidator * CAbstractWifiNetConfigDialog::m_HexValidator = NULL;
	int CAbstractWifiNetConfigDialog::m_HexValidatorRefs = 0;
	
	CAbstractWifiNetConfigDialog::CAbstractWifiNetConfigDialog(libnutwireless::CWireless * interface, QWidget * parent) : QDialog(parent),
		m_ErrorCodeEvaluator(new CErrorCodeEvaluator()),
		m_WifiInterface(interface),
		m_CurrentID(0)
	{
		m_HexValidatorRefs++;
		if (!m_HexValidator) {
			QRegExp regexp("[0123456789abcdefABCDEF]*");
			m_HexValidator = new QRegExpValidator(regexp, this);
		}
	}
	
	CAbstractWifiNetConfigDialog::~CAbstractWifiNetConfigDialog() {
		delete m_ErrorCodeEvaluator;
		
		m_HexValidatorRefs--;
		if (!m_HexValidatorRefs && m_HexValidator) {
			delete m_HexValidator;
			m_HexValidator = NULL;
		}
	}
	
	bool CAbstractWifiNetConfigDialog::execute() {
		m_Config = libnutwireless::CNetworkConfig();
		m_OldConfig = m_Config;
		
		m_CurrentID = -1;
		
		populateUi();
		return exec();
	}
	
	bool CAbstractWifiNetConfigDialog::execute(ScanResult scanResult) {
		m_Config = libnutwireless::CNetworkConfig(scanResult);
		m_OldConfig = m_Config;
		
		m_CurrentID = -1;
		
		populateUi();
		return exec();
	}
	
	bool CAbstractWifiNetConfigDialog::execute(int id) {
		m_Config = m_WifiInterface->getWpaSupplicant()->getNetworkConfig(id);
		m_OldConfig = m_Config;
		
		m_CurrentID = id;
		
		populateUi();
		return exec();
	}
	
	void CAbstractWifiNetConfigDialog::accept() {
		if (applyConfiguration())
			QDialog::accept();
	}
	
	void CAbstractWifiNetConfigDialog::resetUi() {
		m_Config = m_OldConfig;
		populateUi();
	}
	
	#define FLAG_PREPARE_OUTPUT(a, b, c) if(a & c) b << #c;
	
	void CAbstractWifiNetConfigDialog::getConfigErrors(libnutwireless::NetconfigStatus * status, QStringList & errormsg) {
		if (status->failures != NCF_NONE) {
// 			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_ALL)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_SSID)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_BSSID)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_DISABLED)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_ID_STR)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_SCAN_SSID)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_PRIORITY)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_MODE)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_FREQ)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_PROTO)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_KEYMGMT)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_AUTH_ALG)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_PAIRWISE)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_GROUP)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_PSK)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_EAPOL_FLAGS)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_MIXED_CELL)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_PROA_KEY_CACHING)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_WEP_KEY0)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_WEP_KEY1)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_WEP_KEY2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_WEP_KEY3)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_WEP_KEY_IDX)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, NCF_PEERKEY)
		}
		
		if (status->eap_failures != ENCF_NONE) {
// 			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_ALL)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_EAP)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_IDENTITY)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_ANON_IDENTITY)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PASSWD)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_CA_CERT)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_CA_PATH)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_CLIENT_CERT)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PRIVATE_KEY)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PRIVATE_KEY_PASSWD)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_DH_FILE)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_SUBJECT_MATCH)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_ALTSUBJECT_MATCH)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PHASE1)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PHASE2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_CA_CERT2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_CA_PATH2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_CLIENT_CERT2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PRIVATE_KEY2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PRIVATE_KEY2_PASSWD)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_DH_FILE2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_SUBJECT_MATCH2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_ALTSUBJECT_MATCH2)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_FRAGMENT_SIZE)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_EAPPSK)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_NAI)
			FLAG_PREPARE_OUTPUT(status->failures, errormsg, ENCF_PAC_FILE)
		}
	}
	
	void CAbstractWifiNetConfigDialog::convertLineEditText(QLineEdit * lineEdit, bool hex) {
		if (hex) {
			lineEdit->setText(lineEdit->text().toAscii().toHex());
			lineEdit->setValidator(m_HexValidator);
		}
		else {
			lineEdit->setText(QByteArray::fromHex(lineEdit->text().toAscii()));
			lineEdit->setValidator(NULL);
		}
	}
	
	void CAbstractWifiNetConfigDialog::convertLineEditText(bool hex) {
		QCheckBox * hexCheck = qobject_cast<QCheckBox *>(sender());
		if (hexCheck/* && m_HexEditMap.contains(hexCheck)*/)
			convertLineEditText(m_HexEditMap[hexCheck], hex);
	}
	
}
#endif

