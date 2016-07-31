#include "cwireless.h"

#include "wpa_supplicant.h"
#include "cwexthw.h"
#include "cnl80211.h"

namespace libnutwireless {
	CWireless::CWireless(QObject* parent, QString ifname)
	: QObject(parent), m_ifname(ifname) {
		m_wpa_supplicant.reset(new CWpaSupplicant(this, ifname));
		m_wireless_hw.reset(new CWextHW(this,ifname));
		// m_wireless_hw.reset(new libnutwireless::CNL80211(this, m_ifname));
		connect(m_wpa_supplicant.get(), &CWpaSupplicant::message, this, &CWireless::message);
		connect(m_wireless_hw.get(), &CWextHW::message, this, &CWireless::message);
	}

	CWireless::~CWireless() {
		close();
	}

	void CWireless::scan() {
		if (m_wpa_supplicant) m_wpa_supplicant->scan();
		if (m_wireless_hw) m_wireless_hw->scan();
	}

	void CWireless::open() {
		if (!m_wpa_supplicant) {
			m_wpa_supplicant.reset(new libnutwireless::CWpaSupplicant(this, m_ifname));
		}
		m_wpa_supplicant->open();
		if (!m_wireless_hw) {
			// TODO:Add code to switch between wext and new wlan stack as soon as new wlan stack is implemented
			m_wireless_hw.reset(new libnutwireless::CWextHW(this, m_ifname));
			// m_wireless_hw.reset(new libnutwireless::CNL80211(this, m_ifname));
		}
		m_wireless_hw->open();
	}

	void CWireless::close() {
		if (m_wpa_supplicant) m_wpa_supplicant->close();
		if (m_wireless_hw) m_wireless_hw->close();
	}
}
