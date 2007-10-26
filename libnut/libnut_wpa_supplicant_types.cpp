#include "libnut_wpa_supplicant_types.h"
wps_network_config::wps_network_config() {
			//Set default values
			ssid = QString();
			bssid = nut::MacAdress;
			disabled = 0;
			id_str = QString();
			scan_ssid = 0; // (do not) scan with SSID-specific Probe Request frames
			priority = 0;
			mode = 0; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			frequency = 0; //no default, but 0 is not a working value
			wps_protocols proto; //list of accepted protocols TODO: implement
			key_mgmt = KEYMGMT_UNDEFINED; // list of accepted authenticated key management protocols
			wps_auth_algs auth_alg; //list of allowed IEEE 802.11 authentication algorithms TODO:implement
			pairwise = CI_UNDEFINED; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			group = CI_UNDEFINED; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk = QString(); //WPA preshared key; 256-bit pre-shared key
			EAPOL_FLAGS eapol_flags; // IEEE 802.1X/EAPOL options (bit field) TODO:implement
			mixed_cell = false; //This option can be used to configure whether so called mixed
			proactive_key_caching = false,; //Enable/disable opportunistic PMKSA caching for WPA2.
			wep_key0 = QString(); //Static WEP key (ASCII in double quotation, hex without)
			wep_key1 = QString();
			wep_key2 = QString();
			wep_key3 = QString();
			wep_tx_keyidx = WKI_UNDEFINED; //Default WEP key index (TX) (0..3) TODO: implement
			peerkey = false; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
}
wps_network_config::~wps_network_config() {
}