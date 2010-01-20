#include "cwexthw.h"
#include "conversion.h"

extern "C" {
#include <linux/wireless.h>
#include "wpa_ctrl/wpa_ctrl.h"
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}

namespace libnutwireless {

void CWextHW::parseWextIeWpa(unsigned char * iebuf, int buflen, WextRawScan * scan) {

	int ielen = iebuf[1] + 2;
	int offset = 2;	/* Skip the IE id, and the length. */
	unsigned char wpa1_oui[3] = {0x00, 0x50, 0xf2};
	unsigned char wpa2_oui[3] = {0x00, 0x0f, 0xac};
	unsigned char * wpa_oui;
	int i;
	uint16_t ver = 0;
	uint16_t cnt = 0;
	
	if (ielen > buflen) {
		ielen = buflen;
	}
	
	//set wpa_oui
	switch (iebuf[0]) {
		case 0x30:		/* WPA2 */
			/* Check if we have enough data */
			if(ielen < 4) { //Unknown data, return
				return;
			}
			wpa_oui = wpa2_oui;
			break;
	
		case 0xdd:		/* WPA or else */
			wpa_oui = wpa1_oui;
	
			/* Not all IEs that start with 0xdd are WPA. 
			* So check that the OUI is valid. Note : offset==2 */
			if ((ielen < 8) || (memcmp(&iebuf[offset], wpa_oui, 3) != 0) || (iebuf[offset + 3] != 0x01)) {
				return;
			}

			/* Skip the OUI type */
			offset += 4;
			break;
		
		default:
			return;
	}
	
	/* Pick version number (little endian) */
	ver = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;
	
	//Set protocoltype
	if (iebuf[0] == 0xdd) { //WPA1
		scan->protocols = (Protocols) (scan->protocols | PROTO_WPA);
	}
	if (iebuf[0] == 0x30) { //WPA2 "IEEE 802.11i/WPA2 Version"
		scan->protocols = (Protocols) (scan->protocols | PROTO_RSN);
	}
	/* From here, everything is technically optional. */
	
	/* Check if we are done */
	if (ielen < (offset + 4)) {
		/* We have a short IE.  So we should not assume TKIP/TKIP, just return */
		return;
	}
	
	/* Next we have our group cipher. */
	if (memcmp(&iebuf[offset], wpa_oui, 3) != 0) {
// 			printf("                        Group Cipher : Proprietary\n");
	}
	else {
		//Set GroupCiphers
		qDebug() << "GroupCiphers for " << scan->ssid << scan->bssid.toString() << iebuf[offset+3];
		switch (iebuf[offset+3]) {
			case 0:
				scan->group = GCI_NONE;
				break;
			case 1:
				scan->group = GCI_WEP40;
				break;
			case 2:
				scan->group = GCI_TKIP;
				break;
			case 3:
				scan->group = GCI_WRAP;
				break;
			case 4:
				scan->group = GCI_CCMP;
				break;
			case 5:
				scan->group = GCI_WEP104;
				break;
			default:
				scan->group = GCI_UNDEFINED;
				break;
		}
	}
	offset += 4;
	
	/* Check if we are done */
	if (ielen < (offset + 2)) {
		/* We don't have a pairwise cipher, or auth method. DO NOT Assume TKIP. */
		return;
	}
	
	/* Otherwise, we have some number of pairwise ciphers. */
	cnt = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;

	if (ielen < (offset + 4*cnt)) {
		return;
	}
	
	for(i = 0; i < cnt; i++) {
		if (memcmp(&iebuf[offset], wpa_oui, 3) != 0) {
// 				printf(" Proprietary");
		}
		else { //Set PairwiseCiphers
			qDebug() << "PairwiseCiphers for " << scan->ssid << scan->bssid.toString() << iebuf[offset+3];
			switch (iebuf[offset+3]) {
				case 0: scan->pairwise = PCI_NONE; break;
				case 2: scan->pairwise = (PairwiseCiphers) (scan->pairwise | PCI_TKIP); break;
				case 4: scan->pairwise = (PairwiseCiphers) (scan->pairwise | PCI_CCMP); break;
				default: scan->pairwise = (PairwiseCiphers) (scan->pairwise | PCI_UNDEFINED); break;
			}
		}
		offset+=4;
	}
	
	/* Check if we are done */
	if (ielen < (offset + 2)) {
		return;
	}
	
	/* Now, we have authentication suites. */
	cnt = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;

	
	if (ielen < (offset + 4*cnt)) {
		return;
	}
	
	for(i = 0; i < cnt; i++) {
		if(memcmp(&iebuf[offset], wpa_oui, 3) != 0) { //don't care about proprietary
// 				printf(" Proprietary");
		}
		else { //Set the authsuites
			qDebug() << "Setting AUTHSUITES of" << scan->ssid << scan->bssid.toString();
			if (KM_NONE == scan->keyManagement) {
				qDebug() << "Resetting keymanagement";
				scan->keyManagement = KM_UNDEFINED;
			}
			switch (iebuf[offset+3]) {
				case 0:
					if ( OPM_ADHOC == scan->opmode) {
						scan->keyManagement = (KeyManagement) (scan->keyManagement | KM_WPA_NONE);
						qDebug() << "AUTHSUITE: WPA_NONE";
					}
					else {
						scan->keyManagement = (KeyManagement) (scan->keyManagement | KM_NONE);
						qDebug() << "AUTHSUITE: NONE";
					}
					break;
				case 1:
					scan->keyManagement = (KeyManagement) (scan->keyManagement | KM_WPA_EAP | KM_IEEE8021X);
					qDebug() << "AUTHSUITE: WPA_EAP";
					break;
				case 2:
					scan->keyManagement = (KeyManagement) (scan->keyManagement | KM_WPA_PSK);
					qDebug() << "AUTHSUITE: WPA_PSK";
					break;
				default:
					break;
			}
		}
		offset+=4;
	}
	
	/* Check if we are done */
	if (ielen < (offset + 1)) {
		return;
	}
}

SignalQuality CWextHW::convertValues(WextRawScan &scan) {
	SignalQuality res;
	//Set all non-signalquality info
	res.frequency = scan.freq;
	res.bitrates = scan.bitrates;
	res.ssid = scan.ssid;
	res.bssid = scan.bssid;
// 	res.encoding = WSIG_QUALITY_ALLABS;
	qDebug() << "hasRange:" << scan.hasRange;
	if ( scan.hasRange && ((scan.quality.level != 0) || (scan.quality.updated & (IW_QUAL_DBM | IW_QUAL_RCPI))) ) {
		/* Deal with quality : always a relative value */
		if ( !(scan.quality.updated & IW_QUAL_QUAL_INVALID) ) {
			res.quality.value = scan.quality.qual;
			res.quality.maximum = scan.maxquality.qual;
			qDebug() << "Converting: Quality Relative:" << res.quality.value << "from" << scan.quality.qual;
		}

		/* Check if the statistics are in RCPI (IEEE 802.11k) */
		if (scan.quality.updated & IW_QUAL_RCPI) {
		/* Deal with signal level in RCPI */
		/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
			res.type = WSR_RCPI;
			if ( !(scan.quality.updated & IW_QUAL_LEVEL_INVALID) ) {
				res.level.rcpi = ((qreal) scan.quality.level / 2.0) - 110.0;
				qDebug() << "Converting: Level RCPI:" << res.level.rcpi << "from" << scan.quality.level;
			}

			/* Deal with noise level in dBm (absolute power measurement) */
			if ( !(scan.quality.updated & IW_QUAL_NOISE_INVALID) ) {
				res.noise.rcpi = ((qreal) scan.quality.noise / 2.0) - 110.0;
				qDebug() << "Converting: NOISE RCPI:" << res.noise.rcpi << "from" << scan.quality.noise;
			}
		}
		else {
			/* Check if the statistics are in dBm */
			if ( (scan.quality.updated & IW_QUAL_DBM) || (scan.quality.level > scan.maxquality.level) ) {
				res.type = WSR_ABSOLUTE;
				/* Deal with signal level in dBm  (absolute power measurement) */
				if ( !(scan.quality.updated & IW_QUAL_LEVEL_INVALID) ) {
					/* Implement a range for dBm [-192; 63] */
					res.level.nonrcpi.value = (scan.quality.level >= 64) ? scan.quality.level - 0x100 : scan.quality.level;
					qDebug() << "Converting: LEVEL ABS:" << res.level.nonrcpi.value << "from" << scan.quality.level;
				}
			
				/* Deal with noise level in dBm (absolute power measurement) */
				if ( !(scan.quality.updated & IW_QUAL_NOISE_INVALID) ) {

					res.noise.nonrcpi.value = (scan.quality.noise >= 64) ? scan.quality.noise - 0x100 : scan.quality.noise;
					qDebug() << "Converting: NOISE ABS:" << res.noise.nonrcpi.value << "from" << scan.quality.noise;
				}
			}
			else {
				/* Deal with signal level as relative value (0 -> max) */
				res.type = WSR_RELATIVE;
				if ( !(scan.quality.updated & IW_QUAL_LEVEL_INVALID) ) {

					res.level.nonrcpi.value = scan.quality.level;
					res.level.nonrcpi.maximum = scan.maxquality.level;
					qDebug() << "Converting: LEVEL REL:" << res.level.nonrcpi.value << "/" << res.level.nonrcpi.maximum;
				}

				/* Deal with noise level as relative value (0 -> max) */
				if ( !(scan.quality.updated & IW_QUAL_NOISE_INVALID) ) {
					res.noise.nonrcpi.value = scan.quality.noise;
					res.noise.nonrcpi.maximum = scan.maxquality.noise;
					qDebug() << "Converting: NOISE REL:" << res.noise.nonrcpi.value << "/" << res.noise.nonrcpi.maximum;
				}
			}
		}
	}
	else {
		/* We can't read the range, so we don't know... */
		res.type = WSR_UNKNOWN;
		res.quality.value = scan.quality.qual;
		res.quality.maximum = scan.maxquality.qual;
		res.level.nonrcpi.value = scan.quality.level;
		res.level.nonrcpi.maximum = scan.maxquality.level;
		res.noise.nonrcpi.value = scan.quality.noise;
		res.noise.nonrcpi.maximum = scan.maxquality.noise;
		qDebug() << "CONVERTING: ALL UNKNOWN";
	}
	return res;
}

void CWextHW::readSignalQuality() {
	if ( (m_wextFd == -1) ) {
		return;
	}
	struct iw_range range;
	memset(&range,0,sizeof(struct iw_range));
	int hasRange = 0;
	iwstats stats;
	memset(&stats,0,sizeof(iwstats));
	WextRawScan res;
	/* workaround */
	struct wireless_config wifiConfig;
	memset(&wifiConfig,0,sizeof(struct wireless_config));
	/* Get basic information */ 
	if(iw_get_basic_config(m_wextFd, m_ifname.toAscii().constData(), &wifiConfig) < 0) {
		/* If no wireless name : no wireless extensions */ 
		/* But let's check if the interface exists at all */ 
		struct ifreq ifr;
		memset(&ifr,0,sizeof(struct ifreq)); 
	
		strncpy(ifr.ifr_name, m_ifname.toAscii().data(), IFNAMSIZ); 
		if(ioctl(m_wextFd, SIOCGIFFLAGS, &ifr) < 0) 
			qWarning() << tr("(Wireless Extension) No device present");
		else
			qWarning() << tr("(Wireless Extension) device not supported");
		return;
	}
	qDebug() << "Fetched basic config.";

	//We encode frequency in mhz;
	//kernel encodes as double; (hopefully always in hz)
	//But better test this:
	res.freq = -1;
	if (wifiConfig.has_freq) {
		if ( ( (wifiConfig.freq/1e9) < 10.0 ) && ( (wifiConfig.freq/1e9) > 0.0 ) ) {
			res.freq = (int) (wifiConfig.freq/1e6);
		}
	}
	
	struct iwreq wrq;
	memset(&wrq,0,sizeof(struct iwreq));
	
	/* Get AP address */
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
		//Add mac address of current ap;
		res.bssid = libnutcommon::MacAddress((ether_addr*)wrq.u.ap_addr.sa_data);
		qDebug() << "Got AP: " << res.bssid.toString();
	}
	
	/* Get ssid */
	quint8 * buffer = new quint8[IW_ESSID_MAX_SIZE];
	memset(buffer, '\0', IW_ESSID_MAX_SIZE);
	wrq.u.essid.pointer = (void *)buffer;
	wrq.u.essid.length = IW_ESSID_MAX_SIZE;
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWESSID, &wrq) >= 0) {
		if (wrq.u.essid.length > IW_ESSID_MAX_SIZE)
			wrq.u.essid.length = IW_ESSID_MAX_SIZE;
		if (wrq.u.essid.flags) {
			/* Does it have an ESSID index ? */
			if ( wrq.u.essid.pointer && wrq.u.essid.length ) {
				if ( (wrq.u.essid.flags & IW_ENCODE_INDEX) > 1) {
					res.ssid = QString("%1 [%2]").arg(QString::fromAscii((char*) wrq.u.essid.pointer, wrq.u.essid.length), QString::number(wrq.u.essid.flags & IW_ENCODE_INDEX));
				}
				else {
					res.ssid = QString::fromAscii((char*) wrq.u.essid.pointer, wrq.u.essid.length);
				}
			}
			else {
				res.ssid = "N/A";
			}
		}
		qDebug() << "Got ssid: " << res.ssid;
	}
	delete[] buffer;
	
	/* Get bit rate */
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
		res.bitrates.append((qint32) wrq.u.bitrate.value);
		qDebug() << "Got bit rate: " << res.bitrates[0];
	}
	
	/* Get Power Management settings */
	wrq.u.power.flags = 0;
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
		qDebug() << "Got power";
	}
	/* workaround */
	
	
	/* Get range stuff */
	qDebug() << QString("Getting range stuff for %1").arg(m_ifname.toAscii().data());
	if (iw_get_range_info(m_wextFd, m_ifname.toAscii().data(), &range) >= 0) {
		hasRange = 1;
		qDebug() << "Success readWirelessInfo getrange" << strerror(errno);
	}
	else { //This is VERY strange: we always get the "operation not permitted" error, although iw_get_range() worked
		qDebug() << QString("Error \"hasRange == 0\" (%1)").arg(strerror(errno));
	}
	res.hasRange = hasRange;
	if (errno == EAGAIN) {
		m_sqTimeOutCount++;
		qDebug() << QString("Getting range stuff failed (%1)").arg(strerror(errno));
		if ( (m_sqTimeOutCount > 10) && m_sqPollrate < 1000) {
			setSignalQualityPollRate(10000);
		}
		else if (m_sqTimeOutCount > 10) { //Fast polling disabled, but still EAGAIN errors
			//Seems the kernel does not care about our calls
			killTimer(m_sqTimerId);
			m_sqTimerId = -1;
		}
		return;
	}
	
	qDebug() << "Got range stuff";
	

	//Set supported Frequencies if the list is not empty;
	if (m_supportedFrequencies.isEmpty() && hasRange) {
		qDebug() << range.num_frequency;
		qDebug() << "Printing Frequency information";
		quint32 m;
		quint16 e;
		quint32 freqinmhz;
		for (int i=0; i < range.num_channels; i++) {
			m = (quint32) range.freq[i].m;
			e = (quint16) range.freq[i].e;
			freqinmhz = m;
			for (int j=0; j < 9-e-3; j++) {
				freqinmhz = freqinmhz/10;
			}
			if (!m_supportedFrequencies.contains(freqinmhz)) { //Only add frequency that are not in our list
				m_supportedFrequencies.append(freqinmhz);
			}
			qDebug() << m << e << freqinmhz << frequencyToChannel(freqinmhz);
		}
		qDebug() << "Done printing";
	}
	else {
		qDebug() << "m_supportedFrequencies not set";
	}
	
	if (hasRange) {
		res.maxquality.level = (quint8) range.max_qual.level;
		res.maxquality.qual = (quint8) range.max_qual.qual;
		res.maxquality.noise = (quint8) range.max_qual.noise;
		res.maxquality.updated = (quint8) range.max_qual.updated;
		qDebug() << "RANGE: " << res.maxquality.level  << res.maxquality.qual << res.maxquality.noise << res.maxquality.updated;
	}
	else {
		res.maxquality.level = 0;
		res.maxquality.qual = 0;
		res.maxquality.noise = 0;
		res.maxquality.updated = 0;
		qDebug() << "Range information are not available";
	}
	if ( (hasRange) && (range.we_version_compiled > 11) ) {
		struct iwreq wrq;
		memset(&wrq,0,sizeof(struct iwreq));
		wrq.u.data.pointer = (caddr_t) &stats;
		wrq.u.data.length = sizeof(struct iw_statistics);
		wrq.u.data.flags = 1; // Clear updated flag
		strncpy(wrq.ifr_name, m_ifname.toAscii().data(), IFNAMSIZ);
		
		qDebug() << "Getting wireless stats";
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWSTATS, &wrq) < 0) {
			qWarning() << tr("Error occured while fetching wireless info: ") << strerror(errno);
		}
		else { //Stats fetched
			qDebug() << "Stats fetched";
			res.quality.level = (quint8) stats.qual.level;
			res.quality.qual = (quint8) stats.qual.qual;
			res.quality.noise = (quint8) stats.qual.noise;
			res.quality.updated = (quint8) stats.qual.updated;
			qDebug() << "STATS: " << res.quality.level << res.quality.qual << res.quality.noise << res.quality.updated;
			m_sq = convertValues(res);
			qDebug() << "Emittig m_signalQualityUpdated()";
			if (m_sqPollrate < 1000) {
				setSignalQualityPollRate(10000);
				emit message(tr("Auto-resetting timer to 10 seconds"));
			}
			emit signalQualityUpdated(m_sq);
		}
	}
	else if (range.we_version_compiled <= 11) {
		emit message(tr("Cannot fetch wireless information as your wireless extension is too old"));
		emit message(tr("Think about updating your kernel (it's way too old)"));
	}
	else {
		qDebug() << "Error while trying to fetch wireless information" << strerror(errno);
		qDebug() << "Wireless Extension socket file descriptor was: " << m_wextFd;
	}
}

//noise levels, returned by this functions, are invalid, don't use them
void CWextHW::readScanResults() {
	if (m_wextFd == -1) {
		return;
	}
	//Kill Timer:
	if (m_scTimerId != -1) {
		killTimer(m_scPollrate);
		m_scTimerId = -1;
	}
	else { //This is not a timer call; this should not happen
		qWarning() << tr("Warning, no timer present while trying to get scan results");
		return;
	}
	struct iwreq wrq;
	memset(&wrq,0,sizeof(struct iwreq));
	unsigned char * buffer = NULL;		/* Results */
	int buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
	struct iw_range range;
	memset(&range,0,sizeof(struct iw_range));
	int has_range;
	QList<WextRawScan> res = QList<WextRawScan>();
	WextRawScan singleres;
	singleres.bssid = libnutcommon::MacAddress();
	singleres.group = GCI_UNDEFINED;
	singleres.pairwise = PCI_UNDEFINED;
	singleres.protocols = PROTO_UNDEFINED;
	singleres.keyManagement = KM_UNDEFINED;
	
	/* workaround */
	struct wireless_config wifiConfig;
	memset(&wifiConfig,0,sizeof(struct wireless_config));

	/* Get basic information */ 
	if(iw_get_basic_config(m_wextFd, m_ifname.toAscii().constData(), &wifiConfig) < 0) {
		/* If no wireless name : no wireless extensions */ 
		/* But let's check if the interface exists at all */ 
		struct ifreq ifr;
		memset(&ifr,0,sizeof(struct ifreq)); 
	
		strncpy(ifr.ifr_name, m_ifname.toAscii().data(), IFNAMSIZ); 
		if(ioctl(m_wextFd, SIOCGIFFLAGS, &ifr) < 0) 
			qWarning() << tr("(Wireless Extension) No device present");
		else
			qWarning() << tr("(Wireless Extension) Device not supported");
		return;
	}
	qDebug() << "Fetched basic config.";
	
	/* Get AP address */
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
		qDebug() << "Got AP";
	}
	
	/* Get bit rate */
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
		qDebug() << "Got bit rate";
	}
	
	/* Get Power Management settings */
	wrq.u.power.flags = 0;
	if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
		qDebug() << "Got power";
	}
	/* workaround */

	/* Get range stuff */
	/* Get range stuff */
	qDebug() << QString("Getting range stuff for %1").arg(m_ifname.toAscii().data());
	if (iw_get_range_info(m_wextFd, m_ifname.toAscii().data(), &range) >= 0) {
		has_range = 1;
		qDebug() << "Success readWirelessInfo getrange" << strerror(errno);
	}
	else { //This is VERY strange: we always get the "operation not permitted" error, although iw_get_range() worked
		qDebug() << QString("Error \"hasRange == 0\" (%1)").arg(strerror(errno));
	}
	singleres.hasRange = has_range;

	//If we get a timeout for more than 5 times, stop polling.
	if (errno == EAGAIN) {
		qDebug() << "Scan results not available yet";
		m_scTimeOutCount++;
		if (m_scTimeOutCount == 5) {
			return;
		}
		m_scTimerId = startTimer(m_scPollrate);
		return;
	}
	
	qDebug() << "Got range stuff";

	if (has_range) {
		singleres.maxquality.level = (quint8) range.max_qual.level;
		singleres.maxquality.qual = (quint8) range.max_qual.qual;
		singleres.maxquality.noise = (quint8) range.max_qual.noise;
		singleres.maxquality.updated = (quint8) range.max_qual.updated;
		qDebug() << "RANGE (scanresults): " << singleres.maxquality.level  << singleres.maxquality.qual << singleres.maxquality.noise << singleres.maxquality.updated;
	}
	else {
		singleres.maxquality.level = 0;
		singleres.maxquality.qual = 0;
		singleres.maxquality.noise = 0;
		singleres.maxquality.updated = 0;
		qWarning() << tr("Range information are not available");
	}

	unsigned char * newbuf;
	for (int i=0; i <= 16; i++) { //realloc (don't do this forever)
		//Allocate newbuffer 
		newbuf = (uchar*) realloc(buffer, buflen);
		if(newbuf == NULL) {
			if (buffer) {
				free(buffer);
				buffer = NULL;
			}
			qDebug() << "Allocating buffer for Wext failed";
			return;
		}
		buffer = newbuf;
		
		//Set Request variables:
		wrq.u.data.pointer = buffer;
		wrq.u.data.flags = 0;
		wrq.u.data.length = buflen;
		
		//Get the data:
		if (iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWSCAN, &wrq) < 0) {
			//Buffer is too small
			if((errno == E2BIG) && (range.we_version_compiled > 16)) {
				
				//check if driver gives any hints about scan length:
				if (wrq.u.data.length > buflen) {
					buflen = wrq.u.data.length;
				}
				else { //If not, double the length
					buflen *= 2;
				}
				//Start from the beginning:
				qDebug() << "Buffer was too small";
				continue;
			}
			//No range error occured or kernel has wireless extension < 16
			if (errno == EAGAIN) {
				qDebug() << "Scan results not available yet";
				m_scTimerId = startTimer(m_scPollrate);
				return;
			}
			
			//Bad error occured
			if (buffer) {
				free(buffer);
				buffer = NULL;
			}
			qWarning() << tr("(%1) Failed to read scan data : %2").arg(m_ifname, QString(strerror(errno)));
		}
		else { //Results are there
			break;
		}
	}

	//Now read the data:
	if (wrq.u.data.length) {

		struct iw_event iwe;
		memset(&iwe,0,sizeof(struct iw_event));
		struct stream_descr stream;
		memset(&stream,0,sizeof(struct stream_descr));
		int ret;
		
		//Init event stream
// 		char buffer2[128];
		libnutcommon::MacAddress tmpMac;
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		do {
			/* Extract an event and parse it*/
			memset(&iwe,0,sizeof(struct iw_event)); //valgrind complains about it?
			ret = iw_extract_event_stream(&stream, &iwe, range.we_version_compiled);
			if(ret > 0) {
				//Now parse our scan event:
				switch(iwe.cmd) {
					case SIOCGIWAP:
						//ap_addr has type socketaddr
						//Workaround for macaddress
// 						iw_saether_ntop(&(iwe.u.ap_addr), buffer2);
// 						tmpMac = libnutcommon::MacAddress(QString::fromAscii(buffer2,128));
						tmpMac = libnutcommon::MacAddress( (ether_addr*) iwe.u.ap_addr.sa_data);
						if (singleres.bssid.zero()) { //First bssid
							singleres.bssid = tmpMac;
							qDebug() << "Start parsing one network" << singleres.bssid.toString();
						}
						// if our last bssid is different from the actual one, then we have to append it to our scanlist
						if ( (singleres.bssid != tmpMac) )  {
							res.append(singleres);
							qDebug() << "End parsing one network" << singleres.bssid.toString();
							//reset singleres
							singleres.ssid = QString();
							singleres.bssid.clear();
							singleres.quality = WextRawSignal();
							singleres.freq = -1;
							singleres.group = GCI_UNDEFINED;
							singleres.pairwise = PCI_UNDEFINED;
							singleres.keyManagement = KM_UNDEFINED;
							singleres.protocols = PROTO_UNDEFINED;
							singleres.opmode = OPM_AUTO;
							singleres.bssid = tmpMac;
							qDebug() << "Start parsing one network" << singleres.bssid.toString();
						}
						qDebug() << "BSSID" << singleres.bssid.toString();
						break;
					case IWEVQUAL: //Quality event:
						qDebug() << "Quality" << singleres.bssid.toString();
						singleres.quality.qual = (quint8) iwe.u.qual.qual;
						singleres.quality.level = (quint8) iwe.u.qual.level;
						singleres.quality.noise = (quint8) iwe.u.qual.noise;
						singleres.quality.updated = (quint8) iwe.u.qual.updated;
						qDebug() << "STATS (scanresults): " << singleres.quality.level << singleres.quality.qual << singleres.quality.noise << singleres.quality.updated;
						break;
					case SIOCGIWFREQ:
						qDebug() << "Frequency" << singleres.bssid.toString();
						{
							double freq;
							freq = iw_freq2float(&(iwe.u.freq)); //Hopefully in hz
							if ( ( (freq/1e9) < 10.0 ) && ( (freq/1e9) > 0.0 ) ) {
								singleres.freq = (int) (freq/1e6);
								qDebug() << "Channel for" << singleres.ssid << "is " << freq << singleres.freq;
							}
							else {
								singleres.freq = -1;
							}
						}
						break;
					
					case SIOCGIWMODE:
						qDebug() << "Mode:" << singleres.bssid.toString();
						if(iwe.u.mode >= IW_NUM_OPER_MODE) {
							iwe.u.mode = IW_NUM_OPER_MODE;
						}
						singleres.opmode = (OPMODE) iwe.u.mode;
						break;
					case SIOCGIWESSID:
						qDebug() << "ESSID:" << singleres.bssid.toString();
						if (iwe.u.essid.flags) {
							/* Does it have an ESSID index ? */
							if ( iwe.u.essid.pointer && iwe.u.essid.length ) {
								if ( (iwe.u.essid.flags & IW_ENCODE_INDEX) > 1) {
									singleres.ssid = QString("%1 [%2]").arg(QString::fromAscii((char*) iwe.u.essid.pointer,iwe.u.essid.length), QString::number(iwe.u.essid.flags & IW_ENCODE_INDEX));
								}
								else {
									singleres.ssid = QString("%1").arg(QString::fromAscii((char*) iwe.u.essid.pointer,iwe.u.essid.length));
								}
							}
							else {
								singleres.ssid = QString("N/A");
							}
						}
						else { //Hidden essid or broken driver
							singleres.ssid = QString();
						}
						break;

					case SIOCGIWENCODE: //Get encrytion stuff: (just the key)
						qDebug() << "Encode" << singleres.bssid.toString();
						if (! iwe.u.data.pointer) {
							iwe.u.data.flags |= IW_ENCODE_NOKEY;
						}
						if(iwe.u.data.flags & IW_ENCODE_DISABLED) { //Encryption is disabled
							singleres.keyManagement = KM_OFF;
							qDebug() << "PARING ENCODE-Information: NO KEY";
						}
						else {
							//Extract key information: (See iwlib.c line 1500)
							
							//keyflags: iwe.u.data.flags
							//keysize = iwe.u.data.length
							//
							//Do we have a key
							if (iwe.u.data.flags & IW_ENCODE_NOKEY) { //no key
								if (iwe.u.data.length <= 0) {
									//Encryption is on, but group is unknown
									singleres.keyManagement = KM_NONE;
									qDebug() << "PARSING ENCODE-INFORMATION: WEP KEY";
								}
							} //else: we have a, key but check type later
						}
						break;

					case SIOCGIWRATE:
						singleres.bitrates.append((qint32) iwe.u.bitrate.value);
						qDebug() << "Adding Bitrate: " << (qint32) iwe.u.bitrate.value;
						break;
					
					case IWEVGENIE: //group/pairwsie ciphers etc.
						//buffer = iwe.u.data.pointer
						//bufflen = iwe.u.data.length
						qDebug() << "IE_START" << singleres.bssid.toString();
						{
							int offset = 0;
							/* Loop on each IE, each IE is minimum 2 bytes */
							while(offset <= (iwe.u.data.length - 2)) {
								/* Check IE type */
								if (0xdd == ((uchar *) iwe.u.data.pointer)[offset] || (0x30 == ((uchar *) iwe.u.data.pointer)[offset]) ) { // WPA1/2
									parseWextIeWpa(((uchar *) iwe.u.data.pointer) + offset, iwe.u.data.length, &singleres);

								qDebug() << "Parsed IE-Information of " << singleres.ssid << singleres.bssid.toString();
								qDebug() << toString(singleres.group) << toString(singleres.pairwise) << toString(singleres.keyManagement);
								}
								/* Skip over this IE to the next one in the list. */
								offset += buffer[offset+1] + 2;
							}
						}
						qDebug() << "IE_END" << singleres.bssid.toString();
						break;

					default: //Ignore all other event types. Maybe we need them later?
						break;
				}
			}
			else { //Append last scan:
				if (!singleres.bssid.zero()) {
					res.append(singleres);
					qDebug() << "End parsing one network" << singleres.bssid.toString();
				}
			}
	
		} while(ret > 0);

		//Delete buffer
		if (buffer) {
			free(buffer);
			buffer = NULL;
		}
		//We have the data, now construct complete ScanResult
		setScanResults(res);
	}
	else {
		qWarning() << tr("No Scanresults available");
	}
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
}

void CWextHW::setScanResults(QList<WextRawScan> wextScanResults) {
	m_scanResults.clear();
	ScanResult scanresult;
	foreach(WextRawScan i, wextScanResults) {
		scanresult.bssid = i.bssid;
		scanresult.ssid = i.ssid;
		scanresult.freq = i.freq;
		scanresult.signal = convertValues(i);
		scanresult.group = i.group;
		scanresult.pairwise = i.pairwise;
		scanresult.keyManagement = i.keyManagement;
		scanresult.protocols = i.protocols;
		scanresult.opmode = i.opmode;
		scanresult.bitrates = i.bitrates;
		m_scanResults.append(scanresult);
	}

	emit(scanCompleted());
}

}
