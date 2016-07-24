#include "cnl80211.h"

#include "conversion.h"

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <linux/nl80211.h>

#include <QSocketNotifier>
#include <QTimerEvent>
#include <QtEndian>

#include <functional>

#include <libnutnetlink/netlink_msg.h>

namespace {
	// subset from https://github.com/torvalds/linux/blob/master/include/linux/ieee80211.h
	enum ieee80211_eid {
		WLAN_EID_SSID = 0,
		WLAN_EID_SUPP_RATES = 1,
		WLAN_EID_DS_PARAMS = 3,
		WLAN_EID_CF_PARAMS = 4,
		WLAN_EID_TIM = 5,
		WLAN_EID_IBSS_PARAMS = 6,
		WLAN_EID_COUNTRY = 7,
		WLAN_EID_ERP_INFO = 42,
		WLAN_EID_HT_CAPABILITY = 45,
		WLAN_EID_RSN = 48,
		WLAN_EID_EXT_SUPP_RATES = 50,
		WLAN_EID_AP_CHAN_REPORT = 51,
		WLAN_EID_NEIGHBOR_REPORT = 52,
		WLAN_EID_HT_OPERATION = 61,
		WLAN_EID_VENDOR_SPECIFIC = 221,
	};
} // anonymous namespace

namespace libnutwireless {
	CNL80211::CNL80211(QObject* parent, QString ifname)
	: CWirelessHW(parent)
	, m_ifname(ifname)
	{
	}

	CNL80211::~CNL80211(){
		close();
	}

	void CNL80211::timerEvent(QTimerEvent* event) {
		if (event->timerId() == m_sqTimer.timerId()) {
			scan();
			// reset "real" timeout
			m_sqTimer.start(m_sqPollrate, this);
		}
	}

	bool CNL80211::open() {
		if (m_connected)
			return true;

		std::error_code ec;
		m_nlSocket = netlink::generic_netlink_sock::connect(NL80211_GENL_NAME, ec);
		if (ec) {
			emit message(QString("Could not connect NL80211_GENL_NAME: %1").arg(ec.message().c_str()));
			return false;
		}

		m_nlSn.reset(new QSocketNotifier(m_nlSocket.get_socket().get_fd(), QSocketNotifier::Read));
		connect(m_nlSn.get(), &QSocketNotifier::activated, this, &CNL80211::readNlMessage);

		//Start Signal Quality polling
		m_sqTimer.start(0, this);

		m_connected = true;

		return true;
	}

	void CNL80211::close() {
		m_nlSn.reset();
		m_nlSocket.reset();
		m_sqTimer.stop();
		m_connected = false;
	}

	void CNL80211::readNlMessage() {
		//this function should call the appropriate callback functions
		::nl_recvmsgs_default(m_nlSocket.get_socket().get());
	}

	//scan netlink messages are nested (and may even be multipart) (see nl80211.c line 2591: nl80211_send_bss)
	int CNL80211::parseNlScanResult(::nl_msg* msg) {
		::nlmsghdr* msg_hdr = ::nlmsg_hdr(msg);
		::genlmsghdr* msg_header = (::genlmsghdr*) ::nlmsg_data(msg_hdr);

		//Parse the complete message
		::nlattr* attr_buffer[NL80211_ATTR_MAX + 1];
		::nla_parse(attr_buffer, NL80211_ATTR_MAX, ::genlmsg_attrdata(msg_header, 0), ::genlmsg_attrlen(msg_header, 0), NULL);

		if (!attr_buffer[NL80211_ATTR_BSS]) { //Check if BSS
			return NL_OK;
		}

		::nlattr* bss_buffer[NL80211_BSS_MAX + 1]; // BSS (basic service set)
		// This is the struct to check the validity of the attributes. See enum nl80211_bss
		struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {};
		bss_policy[NL80211_BSS_BSSID].minlen = 6;
		bss_policy[NL80211_BSS_FREQUENCY].type = NLA_U32;
		bss_policy[NL80211_BSS_TSF].type = NLA_U64;
		bss_policy[NL80211_BSS_BEACON_INTERVAL].type = NLA_U16;
		bss_policy[NL80211_BSS_SIGNAL_MBM].type = NLA_U32;
		bss_policy[NL80211_BSS_SIGNAL_UNSPEC].type = NLA_U8;
		bss_policy[NL80211_BSS_STATUS].type = NLA_U32;

		//Parse the nested attributes. this is where the scan results are
		if (::nla_parse_nested(bss_buffer, NL80211_BSS_MAX, attr_buffer[NL80211_ATTR_BSS], bss_policy)) {
			return NL_OK;
		}

		if (!bss_buffer[NL80211_BSS_BSSID]) return NL_SKIP;

		ScanResult scanresult;

		scanresult.bssid = libnutcommon::MacAddress((ether_addr*)(bss_buffer[NL80211_BSS_BSSID]));
		scanresult.signal.bssid = scanresult.bssid;

		if (bss_buffer[NL80211_BSS_FREQUENCY]) {
			scanresult.freq = ::nla_get_u32(bss_buffer[NL80211_BSS_FREQUENCY]);
		}

		if (bss_buffer[NL80211_BSS_SIGNAL_MBM]) {
			scanresult.signal.type = WSR_RCPI;
			scanresult.signal.quality.value = ::nla_get_u32(bss_buffer[NL80211_BSS_SIGNAL_MBM])/100;
		}
		else if (bss_buffer[NL80211_BSS_SIGNAL_UNSPEC]) {
			scanresult.signal.type = WSR_UNKNOWN;
			scanresult.signal.quality.value = ::nla_get_u8(bss_buffer[NL80211_BSS_SIGNAL_UNSPEC]);
		}

		bool const active = bss_buffer[NL80211_BSS_STATUS];

		{
			::nlattr* ie =
					bss_buffer[NL80211_BSS_INFORMATION_ELEMENTS]
					? bss_buffer[NL80211_BSS_INFORMATION_ELEMENTS]
					: bss_buffer[NL80211_BSS_BEACON_IES];
			if (!ie) return NL_OK;
			quint8 const* ie_data = reinterpret_cast<quint8 const*>(::nla_data(ie));
			size_t const ie_len = static_cast<size_t>(::nla_len(ie));

			for (size_t ie_pos = 0; (ie_pos + 2 <= ie_len) && (ie_pos + 2 + ie_data[ie_pos+1] <= ie_len); ie_pos += 2 + ie_data[ie_pos+1]) {
				quint8 const field_type = ie_data[ie_pos];
				size_t const field_len = ie_data[ie_pos+1];
				quint8 const* field_data = ie_data + ie_pos + 2;
				size_t field_offset;

				scanresult.opmode = OPM_MANAGED;

				switch (field_type) {
				case WLAN_EID_SSID:
					scanresult.ssid = libnutcommon::SSID::fromRaw(field_data, field_len);
					scanresult.signal.ssid = scanresult.ssid;
					break;
				case WLAN_EID_SUPP_RATES:
				case WLAN_EID_EXT_SUPP_RATES:
					for (size_t i = 0; i < field_len; ++i) {
						quint8 const rate = field_data[i];
						if (0xff == rate) continue; // "HT PHY" BSS membership selector
						// MSB encodes "contained in BSSBasicRateSet"
						scanresult.bitrates.push_back((rate & 0x7f) * 500 * 1000);
					}
					scanresult.signal.bitrates = scanresult.bitrates;
					break;
				case WLAN_EID_DS_PARAMS: // channel, single byte
					break;
				case WLAN_EID_COUNTRY:
					break;
				case WLAN_EID_ERP_INFO:
					break;
				case WLAN_EID_HT_CAPABILITY: // 0x1a bytes
					break;
				case WLAN_EID_RSN:
					field_offset = 0;

					scanresult.protocols = Protocols(scanresult.protocols | PROTO_RSN);
					// overwrite PROTO_WPA details if there are any
					scanresult.group = GCI_CCMP;
					scanresult.pairwise = PCI_CCMP;
					scanresult.keyManagement = KM_IEEE8021X;

					// check version
					if (field_offset + 2 > field_len || 1 != qFromLittleEndian<quint16>(field_data + field_offset)) continue;
					field_offset += 2;

					if (field_offset + 4 <= field_len) {
						scanresult.group = GCI_UNDEFINED;
						switch (qFromBigEndian<quint32>(field_data + field_offset)) {
						case 0x000FAC01:
							scanresult.group = GCI_WEP40;
							break;
						case 0x000FAC02:
							scanresult.group = GCI_TKIP;
							break;
						case 0x000FAC04:
							scanresult.group = GCI_CCMP;
							break;
						case 0x000FAC05:
							scanresult.group = GCI_WEP104;
							break;
						}
						field_offset += 4;
					}
					if (field_offset + 2 <= field_len) {
						scanresult.pairwise = PCI_UNDEFINED;

						quint16 pairwise_count = qFromLittleEndian<quint16>(field_data + field_offset);
						field_offset += 2;

						if (pairwise_count * 4u + field_offset > field_len) continue;

						for (size_t i = 0; i < pairwise_count; ++i, field_offset += 4) {
							switch (qFromBigEndian<quint32>(field_data + field_offset)) {
							case 0x000FAC00:
								scanresult.pairwise = PairwiseCiphers(scanresult.pairwise | PCI_DEF);
								break;
							case 0x000FAC02:
								scanresult.pairwise = PairwiseCiphers(scanresult.pairwise | PCI_TKIP);
								break;
							case 0x000FAC04:
								scanresult.pairwise = PairwiseCiphers(scanresult.pairwise | PCI_CCMP);
								break;
							}
						}
						if (field_offset + 2 < field_len) {
							scanresult.keyManagement = KM_UNDEFINED;
							quint16 auth_count = qFromLittleEndian<quint16>(field_data + field_offset);
							field_offset += 2;
							if (auth_count * 4u + field_offset > field_len) continue;

							for (size_t i = 0; i < auth_count; ++i, field_offset += 4) {
								switch (qFromBigEndian<quint32>(field_data + field_offset)) {
								case 0x000FAC01:
								case 0x000FAC03:
								case 0x000FAC05:
									scanresult.keyManagement = KeyManagement(scanresult.keyManagement | KM_IEEE8021X);
									break;
								case 0x000FAC02:
								case 0x000FAC04:
								case 0x000FAC06:
									scanresult.keyManagement = KeyManagement(scanresult.keyManagement | KM_WPA_PSK);
									break;
								}
							}
						}
					}
					break;
				case WLAN_EID_VENDOR_SPECIFIC:
					if (field_len < 4) continue;
					switch (qFromBigEndian<quint32>(field_data)) {
					case 0x0050F201: // Microsoft WPA
						scanresult.protocols = Protocols(scanresult.protocols | PROTO_WPA);
						if (0 != (PROTO_RSN & scanresult.protocols)) continue; // don't add WPA details when we have WPA2 (RSN)
						field_offset = 4;

						scanresult.group = GCI_TKIP;
						scanresult.pairwise = PCI_TKIP;
						scanresult.keyManagement = KM_IEEE8021X;

						// check version
						if (field_offset + 2 > field_len || 1 != qFromLittleEndian<quint16>(field_data + field_offset)) continue;
						field_offset += 2;

						if (field_offset + 4 <= field_len) {
							scanresult.group = GCI_UNDEFINED;
							switch (qFromBigEndian<quint32>(field_data + field_offset)) {
							case 0x0050F200:
								scanresult.group = GCI_NONE;
								break;
							case 0x0050F201:
								scanresult.group = GCI_WEP40;
								break;
							case 0x0050F202:
								scanresult.group = GCI_TKIP;
								break;
							case 0x0050F204:
								scanresult.group = GCI_CCMP;
								break;
							case 0x0050F205:
								scanresult.group = GCI_WEP104;
								break;
							}
							field_offset += 4;
						}
						if (field_offset + 2 <= field_len) {
							scanresult.pairwise = PCI_UNDEFINED;

							quint16 pairwise_count = qFromLittleEndian<quint16>(field_data + field_offset);
							field_offset += 2;

							if (pairwise_count * 4u + field_offset > field_len) continue;

							for (size_t i = 0; i < pairwise_count; ++i, field_offset += 4) {
								switch (qFromBigEndian<quint32>(field_data + field_offset)) {
								case 0x0050F200:
									scanresult.pairwise = PairwiseCiphers(scanresult.pairwise | PCI_NONE);
									break;
								case 0x0050F202:
									scanresult.pairwise = PairwiseCiphers(scanresult.pairwise | PCI_TKIP);
									break;
								case 0x0050F204:
									scanresult.pairwise = PairwiseCiphers(scanresult.pairwise | PCI_CCMP);
									break;
								}
							}
							if (field_offset + 2 < field_len) {
								scanresult.keyManagement = KM_UNDEFINED;
								quint16 auth_count = qFromLittleEndian<quint16>(field_data + field_offset);
								field_offset += 2;
								if (auth_count * 4u + field_offset > field_len) continue;

								for (size_t i = 0; i < auth_count; ++i, field_offset += 4) {
									switch (qFromBigEndian<quint32>(field_data + field_offset)) {
									case 0x0050F200:
										scanresult.keyManagement = KeyManagement(scanresult.keyManagement | KM_WPA_NONE);
										break;
									case 0x0050F201:
										scanresult.keyManagement = KeyManagement(scanresult.keyManagement | KM_IEEE8021X);
										break;
									case 0x0050F202:
										scanresult.keyManagement = KeyManagement(scanresult.keyManagement | KM_WPA_PSK);
										break;
									}
								}
							}
						}
					}
					break;
				}
			}
		}

		if (active) {
			m_sq = scanresult.signal;
			m_scanFoundCurrentSignal = true;
			emit signalQualityUpdated(m_sq);
		}

		m_accumScanResults.push_back(scanresult);
		return NL_OK;
	}

	void CNL80211::scan() {
		if (!m_connected) {
			emit message("Netlink not connected, cannot scan");
			return;
		}
		if (m_scanRunning) {
			m_scanPending = true;
			return;
		}
		m_scanRunning = true;
		m_scanFoundCurrentSignal = false;

		m_accumScanResults.clear();

		//Get interface index for device:
		unsigned int devidx = if_nametoindex(m_ifname.toLatin1().constData());
		if (devidx == 0) {
			emit message("Could not get interface index");
			return;
		}
		//Allocate a new netlink message
		auto msg = netlink::nl_msg_ref::alloc();
		if (!msg) {
			emit message("Could not allocate netlink message");
			return;
		}

		m_nlSocket.get_socket().get_cb().set(netlink::callback_type::valid, std::bind(&CNL80211::parseNlScanResult, this, std::placeholders::_1));
		m_nlSocket.get_socket().get_cb().set(netlink::callback_type::finish, [this](::nl_msg*) -> int {
			m_scanResults = std::move(m_accumScanResults);
			m_scanRunning = false;
			if (m_scanPending) m_sqTimer.start(0, this);
			if (!m_scanFoundCurrentSignal) {
				// no current signal quality (anymore); clear it
				m_sq = SignalQuality{};
				emit signalQualityUpdated(m_sq);
			}

			emit scanCompleted();
			return NL_OK;
		});

		::genlmsg_put(msg, 0, 0, m_nlSocket.get_family().get_id(), 0, (NLM_F_REQUEST | NLM_F_DUMP), NL80211_CMD_GET_SCAN, 0);
		//Set the interface we want to operate on
		::nla_put_u32(msg, NL80211_ATTR_IFINDEX, devidx);

		//Send the message
		m_nlSocket.get_socket().send_auto(msg);
	}


	QList<ScanResult> CNL80211::getScanResults() {
		return m_scanResults;
	}

	SignalQuality CNL80211::getSignalQuality() {
		return m_sq;
	}

	void CNL80211::setSignalQualityPollRate(int msec) {
		m_sqPollrate = msec;
		m_sqTimer.start(m_sqPollrate, this);
	}

	int CNL80211::getSignalQualityPollRate() {
		return m_sqPollrate;
	}

	QList<quint32> CNL80211::getSupportedChannels() {
		QList<quint32> chans;
		for (qint32 freq: m_supportedFrequencies) {
			qint32 const chan = frequencyToChannel(freq);
			if (chan > 0) chans.append(chan);
		}
		return chans;
	}

	QList<quint32> CNL80211::getSupportedFrequencies() {
		return m_supportedFrequencies;
	}
}
