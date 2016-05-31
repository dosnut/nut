#include "configparser.h"

namespace nuts {
	struct config_tokenvalue {
		QString str;
		QHostAddress addr;
		libnutcommon::MacAddress macAddr;
		int i = 0;
		bool b = false;
	};

	enum config_tokentype: int {
		ENDOFFILE = 0,
		INVALID_TOKEN = 258,
		INVALID_CHAR,
		DEVICE,
		ENVIRONMENT,
		REGEXP,
		NOAUTOSTART,
		DEFAULT,
		DHCP,
		NODHCP,
		ZEROCONF,
		NOZEROCONF,
		STATIC,
		FALLBACK,
		TIMEOUT,
		CONTINUEDHCP,
		IP,
		NETMASK,
		GATEWAY,
		DNSSERVER,
		METRIC,
		LABELINDEX,
		SELECT,
		USER,
		ARP,
		ESSID,
		AND,
		OR,
		WLAN,
		MODE,
		WPASUPPLICANT,
		CONFIG,
		DRIVER,
		STRING,
		IPv4_VAL,
		MACADDR,
		INTEGER,
		BOOL,
	};

	struct config_lloc {
		int lineNum = 1;
	};

	typedef void* config_tokenctx;
}

extern int configparserlex(nuts::config_tokenvalue*, nuts::config_lloc*, nuts::config_tokenctx scanner);
extern int configparserlex_init(nuts::config_tokenctx* scanner);
extern void configparserset_in(FILE* in_str, nuts::config_tokenctx scanner);
extern int configparserlex_destroy(nuts::config_tokenctx scanner);
