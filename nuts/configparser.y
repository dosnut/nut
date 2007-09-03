%{
	#include "config.h"
	#include <QStack>

	extern int configparserlex (void);
	extern int line_num;
	
	using namespace nuts;
	
	static void configparsererror (Config *config, char* s);

	static DeviceConfig *curdevconfig;
	static EnvironmentConfig *curenvconfig;
	static IPv4Config *curipv4config;
	static SelectConfig *curselconfig;
	static QStack<size_t> selBlocks;
	
	static SelectConfig *selectNew() {
		curselconfig = new SelectConfig();
		selBlocks.clear();
		return curselconfig;
	}
	
#define selectAdd(args...) do { \
	size_t filterid = curselconfig->filters.size(); \
	curselconfig->filters.append(SelectFilter(args)); \
	curselconfig->blocks.last().append(filterid); \
} while(0)
	static void selectPushAnd() {
		size_t blockid = curselconfig->blocks.size();
		if (blockid)
			selectAdd(blockid);
		curselconfig->blocks.append(QVector<size_t>(1, 0));
	}
	
	static void selectPushOr() {
		size_t blockid = curselconfig->blocks.size();
		if (blockid)
			selectAdd(blockid);
		curselconfig->blocks.append(QVector<size_t>(1, 1));
	}

%}

%union {
	QString *str;
	QHostAddress *addr;
	nut::MacAddress *macAddr;
	int i;
}

%token DEVICE ENVIRONMENT
%token DEFAULT
%token DHCP NODHCP ZEROCONF NOZEROCONF STATIC
%token IP NETMASK GATEWAY DNSSERVER
%token LABELINDEX
%token SELECT USER ARP ESSID
%token AND OR
%token WLAN MODE WPACONFIG

%token <str> STRING
%token <addr> IPv4_VAL
%token <macAddr> MACADDR
%token <i> INTEGER
%error-verbose

%parse-param {Config *curconfig};
%start input

%%

input:
	| input device
;

device: DEVICE STRING { curdevconfig = curconfig->createDevice(*$2); } deviceconfig
;

deviceconfig: ';'
	| '{' deviceoptions '}'
	| '{' deviceoptions '}' ';'
	| deviceoption
;

deviceoptions:
	| deviceoptions deviceoption
;

deviceoption: { curenvconfig = curdevconfig->getDefaultEnv(); } environmentoption
	| environment
;

environment: ENVIRONMENT { curenvconfig = curdevconfig->createEnvironment(); } environmentconfig
;

environmentconfig: ';'
	| '{' environmentoptions '}'
	| '{' environmentoptions '}' ';'
	| environmentoption
;

environmentoptions:
	| environmentoptions environmentoption
;

environmentoption: dhcpconfig
	| static
	| { if (!curenvconfig->select) curenvconfig->select = selectNew(); else {
		yyerror(curconfig, "only one select block in one environment allowed");
		YYERROR;
	}} select
;

dhcpconfig: DHCP ';' { curenvconfig->doDHCP(); }
;

static: STATIC { curipv4config = curenvconfig->createStatic();  } staticconfig
;

staticconfig: '{' staticoptions '}'
	|  '{' staticoptions '}' ';'
	| staticoption_ip
;

staticoptions:
	| staticoptions staticoption
;

staticoption: staticoption_ip
	| staticoption_netmask
	| staticoption_gateway
	| staticoption_dns
;

staticoption_ip: IP IPv4_VAL ';' { curipv4config->static_ip = *$2; }
;

staticoption_netmask: NETMASK IPv4_VAL ';' { curipv4config->static_netmask = *$2; }
;

staticoption_gateway: GATEWAY IPv4_VAL ';' { curipv4config->static_gateway = *$2; }
;

staticoption_dns: DNSSERVER IPv4_VAL ';' { curipv4config->static_dnsservers.push_back(*$2); }
;

select: selectstart selectblock
;

selectstart: SELECT AND { selectPushAnd(); }
	| SELECT OR { selectPushOr(); }
	| SELECT { selectPushAnd(); }
;

selectblock: '{' selectfilters '}'
	| '{' selectfilters '}' ';'
	| selectfilter
;

selectfilters:
	| selectfilters selectfilter
;

selectfilter: sf_user
	| sf_arp
	| sf_essid
	| sf_block
;

sf_user: USER ';' { selectAdd(); }
;

sf_arp: ARP MACADDR ';' { selectAdd(*$2); }
	| ARP MACADDR IPv4_VAL ';' { selectAdd(*$2, *$3); }
;

sf_essid: ESSID STRING ';' { selectAdd(*$2); }
;

sf_block: select
;

%%

void configparsererror (Config *, char*) {
}
