#include "cnl80211.h"
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <QSocketNotifier>


namespace libnutwireless {

//copied from net-wireless/iw
#ifndef CONFIG_LIBNL20
/* libnl 2.0 compatibility code */

static inline struct nl_handle *nl_socket_alloc(void)
{
	return nl_handle_alloc();
}

static inline void nl_socket_free(struct nl_sock *h)
{
	nl_handle_destroy(h);
}

static inline int __genl_ctrl_alloc_cache(struct nl_sock *h, struct nl_cache **cache)
{
	struct nl_cache *tmp = genl_ctrl_alloc_cache(h);
	if (!tmp)
		return -ENOMEM;
	*cache = tmp;
	return 0;
}
#define genl_ctrl_alloc_cache __genl_ctrl_alloc_cache
#endif /* CONFIG_LIBNL20 */

CNL80211::CNL80211(QObject* parent, QString ifname) :
CWirelessHW(parent),
m_ifname(ifname),
m_connected(false),
m_nlFd(-1),
m_nlSn(0),
m_nlSocket(0),
m_nlCache(0),
m_nlFamily(0),
m_sqPollrate(10000),
m_sqTimeOutCount(0),
m_sqTimerId(-1)
{}
CNL80211::~CNL80211(){
	close();
}

void CNL80211::timerEvent(QTimerEvent *event) {
	if (event->timerId() == m_sqTimerId){
		readSignalQuality();
	}

}

bool CNL80211::open() {
	if (m_connected)
		return;
	m_nlSocket = nl_socket_alloc();
	if(!m_nlSocket) {
		emit message("Could not create netlink socket");
		return false;
	}
	if (!genl_connect(m_nlSocket)) {
		emit message("Could not connect to generic netlink interface");
		close();
		return false;
	}
	if (genl_ctrl_alloc_cache(m_nlSocket, &m_nlCache)) {
		emit message("Could not allocate generic netlink cache.");
		close();
		return false;
	}

	m_nlFamily = genl_ctrl_search_by_name(m_nlCache, "nl80211");
	if (!m_nlFamily) {
		emit message("Could not find nl80211");
		close();
		return false;
	}
	
	m_nlFd = nl_socket_get_fd(m_nlSocket);
	m_nlSn = new QSocketNotifier(m_nlFd,QSocketNotifier::Read,this);
	connect(m_nlSn,SIGNAL(activated(int)), this, SLOT(readNlMessage(void)));
	
	//Start Signal Quality polling
	m_sqTimerId = startTimer(m_sqPollrate);
	
	m_connected = true;
	return true;
}

void CNL80211::close() {
	//don't check if not connected, check pointers
	if (m_nlSn) {
		m_nlSn->setEnabled(false);
		delete m_nlSn;
	}
	if (m_nlFamily)
		genl_family_put(m_nlFamily);
	if (m_nlCache)
		nl_cache_free(m_nlCache);
	if (m_nlSocket)
		nl_socket_free(m_nlSocket);
	m_nlFd = -1;
	m_connected = false;
}

void CNL80211::readNlMessage() {
	//this function should call the apropriate call back functions
	nl_recvmsgs_default(m_nlSocket);
}

//scan netlink messages are nested (and may even be multipart) (see nl80211.c line 2591: nl80211_send_bss)
int CNL80211::parseNlScanResult(nl_msg * msg) {
	struct nlattr * attr_buffer[NL80211_ATTR_MAX + 1];
	struct nlmsghdr * msg_hdr = nlmsg_hdr(msg);
	struct genlmsghdr * msg_header = nlmsg_data(msg_hdr);
	struct nlattr * bss_buffer[NL80211_BSS_MAX + 1]; //bss = basic service set
	ScanResult scanresult;
	//This is the struct to check the validity of the attributes. See enum nl80211_bss
	struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_TSF] = { .type = NLA_U64 },
		[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_BSS_BSSID] = { },
		[NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
		[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
		[NL80211_BSS_INFORMATION_ELEMENTS] = { },
		[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
		[NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
		[NL80211_BSS_STATUS] = { .type = NLA_U32 },
	};
	if (msg_hdr->nlmsg_flags & NLM_F_MULTI)
		qDebug() << "netlink: Mutlipart message";

	//Parse the complete message
	nla_parse(attr_buffer, NL80211_ATTR_MAX, genlmsg_attrdata(msg_header, 0), genlmsg_attrlen(msg_header, 0), NULL);

	if (!attr_buffer[NL80211_ATTR_BSS]) { //Check if BSS
		return NL_SKIP;
	}
	//Parse the nested attributes. this is where the scan results are
	if (nla_parse_nested(bss_buffer, NL80211_BSS_MAX, attr_buffer[NL80211_ATTR_BSS], bss_policy)) {
		return NL_SKIP;
	}

	if (!bss_buffer[NL80211_BSS_BSSID])
		return NL_SKIP;

	scanresult.bssid = libnutcommon::MacAddress((ether_addr*)(bss_buffer[NL80211_BSS_BSSID]));
	scanresult.signal.bssid = scanresult.bssid;
	
	if (bss_buffer[NL80211_BSS_FREQUENCY])
		scanresult.freq = nla_get_u32(bss_buffer[NL80211_BSS_FREQUENCY]);

	if (bss_buffer[NL80211_BSS_SIGNAL_MBM]) {
		scanresult.signal.type = WSR_RCPI;
		scanresult.signal.quality.value = nla_get_u32(bss_buffer[NL80211_BSS_SIGNAL_MBM])/100;
	}
	if (bss_buffer[NL80211_BSS_SIGNAL_UNSPEC]) {
		scanresult.signal.type = WSR_UNKNOWN;
		scanresult.signal.quality.value = nla_get_u8(bss_buffer[NL80211_BSS_SIGNAL_UNSPEC]);
	}
	if (bss_buffer[NL80211_BSS_INFORMATION_ELEMENTS])
		print_ies(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]),
			  nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]),
			  params->unknown, params->type);

	return NL_SKIP;
}

static int cbForScanResults(struct nl_msg *msg, void *arg) {
	libnutwireless::CNL80211 * obj = dynamic_cast<libnutwireless::CNL80211>(arg);
	if (!obj)
		return NL_SKIP;
	return obj->parseNlScanResult(nl_msg * msg);
}

void CNL80211::scan() {
	//Get interface index for device:
	unsigned int devidx = if_nametoindex(m_ifname.toAscii().constData());
	if (devidx == 0) {
		emit message("Could not get interface index");
		return;
	}
	//Allocate a new netlink message
	nl_msg * msg;
	msg = nlmsg_alloc();
	if (!msg) {
		emit message("Could not allocate netlink message");
		return;
	}
	
	//allocate the callback function with default verbosity
// 	nl_cb * cb = nl_cb_alloc(NL_CB_DEFAULT);
// 	if (!cb) {
// 		emit message("Could not allocate netlink callback");
// 		nlmsg_free(msg);
// 		return;
// 	}

	nl_socket_modify_cb(m_nlSocket, NL_CB_VALID, NL_CB_CUSTOM, cbForScanResults, this);
	
	genlmsg_put(msg, 0, 0, genl_family_get_id(m_nlFamily), 0, (NLM_F_REQUEST | NLM_F_DUMP), NL80211_CMD_GET_SCAN, 0);
	//Set the interface we want to operate on
	nla_put_u32(msg, NL80211_ATTR_IFNAME,devidx);
	
	//hier kommt noch was rein
	
	//Send the message
	nl_send_auto_complete(m_nlSocket,msg);
	nlmsg_free(msg);
}


QList<ScanResult> CNL80211::getScanResults() {
	return m_scanResults;
}

SignalQuality CNL80211::getSignalQuality() {
	return m_sq;
}

void CNL80211::setSignalQualityPollRate(int msec) {
	m_sqTimeOutCount = 0;
	if (m_sqTimerId != -1) {
		killTimer(m_sqTimerId);
	}
	m_sqPollrate = msec;
	m_sqTimerId = startTimer(m_sqPollrate);
}

int CNL80211::getSignalQualityPollRate() {
	return m_sqPollrate;
}

QList<quint32> CNL80211::getSupportedChannels() {
	QList<quint32> chans;
	qint32 chan;
	foreach(qint32 freq, m_supportedFrequencies) {
		chan = frequencyToChannel(freq);
		if (chan > 0)
			chans.append(chan);
	}
	return  chans;
}

QList<quint32> CNL80211::getSupportedFrequencies() {
	return m_supportedFrequencies;
}
	
	
}