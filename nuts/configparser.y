%{
	#include "config.h"
	

	extern int configparserlex (void);
	extern int line_num;
	
	using namespace nuts;
	
	static void configparsererror (ConfigParser *cp, const char* s);
	
	#define CHECK(action) do { if (!(cp->action)) YYERROR; } while (0)
%}

%union {
	QString *str;
	QHostAddress *addr;
	nut::MacAddress *macAddr;
	int i;
}

%token DEVICE ENVIRONMENT
%token NOAUTOSTART
%token DEFAULT
%token DHCP NODHCP ZEROCONF NOZEROCONF STATIC
%token IP NETMASK GATEWAY DNSSERVER
%token LABELINDEX
%token SELECT USER ARP ESSID
%token AND OR
%token WLAN MODE
 
%token WPASUPPLICANT CONFIG DRIVER

%token <str> STRING
%token <addr> IPv4_VAL
%token <macAddr> MACADDR
%token <i> INTEGER

%destructor { delete ($$); } STRING IPv4_VAL MACADDR

%error-verbose

%parse-param {ConfigParser *cp};
%start input

%%

input:
	| input device
;

device: DEVICE STRING { CHECK(newDevice(*$2)); delete $2; } deviceconfig { CHECK(finishDevice()); }
;

deviceconfig: ';'
	| '{' deviceoptions '}'
	| '{' deviceoptions '}' ';'
	| deviceoption
;

deviceoptions:
	| deviceoptions deviceoption
;

deviceoption: { CHECK(devDefaultEnvironment()); } environmentoption
	| environment
	| wpasupplicant
	| NOAUTOSTART { CHECK(devNoAutoStart()); } ';'
;

wpasupplicant: WPASUPPLICANT DRIVER STRING CONFIG STRING ';' { CHECK(devWPASuppConfig(*$3, *$5)); delete $3; delete $5; }
	| WPASUPPLICANT CONFIG STRING ';' { CHECK(devWPASuppConfig("wext", *$3)); delete $3; }
	| WPASUPPLICANT CONFIG STRING DRIVER STRING ';' { CHECK(devWPASuppConfig(*$5, *$3)); delete $5; delete $3; }
;

environment: ENVIRONMENT STRING { CHECK(devEnvironment(*$2)); } environmentconfig { CHECK(finishEnvironment()); delete $2; }
	| ENVIRONMENT { CHECK(devEnvironment("")); } environmentconfig { CHECK(finishEnvironment()); }
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
	| { CHECK(envSelect()); } select { CHECK(finishSelect()); }
	| NODHCP { CHECK(envNoDHCP()); }
;

dhcpconfig: DHCP { CHECK(envDHCP()); } ';' { CHECK(finishDHCP()); }
;

static: STATIC { CHECK(envStatic());  } staticconfig { CHECK(finishStatic()); }
	| STATIC USER  { CHECK(envStatic()); CHECK(staticUser()); } ';'  { CHECK(finishStatic()); }
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

staticoption_ip: IP IPv4_VAL ';' { CHECK(staticIP(*$2)); delete $2; }
;

staticoption_netmask: NETMASK IPv4_VAL ';' { CHECK(staticNetmak(*$2)); delete $2; }
;

staticoption_gateway: GATEWAY IPv4_VAL ';' { CHECK(staticGateway(*$2)); delete $2; }
;

staticoption_dns: DNSSERVER IPv4_VAL ';' { CHECK(staticDNS(*$2)); delete $2; }
;

select: selectstart selectblock
;

selectstart: SELECT AND { CHECK(selectAndBlock()); }
	| SELECT OR { CHECK(selectOrBlock()); }
	| SELECT { CHECK(selectAndBlock()); }
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
	| error
;

sf_user: USER ';' { CHECK(selectUser()); }
;

sf_arp: ARP IPv4_VAL ';' { CHECK(selectARP(*$2)); delete $2; }
	| ARP IPv4_VAL MACADDR ';' { CHECK(selectARP(*$2, *$3)); delete $2; delete $3; }
;

sf_essid: ESSID STRING ';' { CHECK(selectESSID(*$2)); delete $2; }
;

sf_block: select
;

%%

void configparsererror (ConfigParser *cp, const char* msg) {
	cp->parseError(line_num, QString::fromUtf8(msg));
}
