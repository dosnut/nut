%{
	#include "config.h"
	

	extern int configparserlex (void);
	extern int line_num;
	
	using namespace nuts;
	
	static void configparsererror (ConfigParser *cp, const char* s);
	
	//Check if invoked action worked (e.g. newDevice -> device creation worked)
	#define CHECK(action) do { if (!(cp->action)) YYERROR; } while (0)
%}

%union {
	QString *str;
	QHostAddress *addr;
	libnutcommon::MacAddress *macAddr;
	int i;
	bool b;
}

%token DEVICE ENVIRONMENT
%token NOAUTOSTART
%token DEFAULT
%token DHCP NODHCP ZEROCONF NOZEROCONF STATIC 
%token FALLBACK TIMEOUT CONTINUEDHCP
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
%token <b> BOOL

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
	| zeroconf
	| static
	| SELECT { CHECK(envSelect()); } select { CHECK(finishSelect()); }
	| NODHCP { CHECK(envNoDHCP()); }
;

dhcpconfig: DHCP { CHECK(envDHCP()); } ';' { CHECK(finishDHCP()); }
	| DHCP { CHECK(envDHCP()); } FALLBACK { CHECK(envFallback()); } fallbackconfig { CHECK(finishFallback()); } { CHECK(finishDHCP()); }
;

fallbackconfig: fallbackoption
	| '{' fallbackoptionbracket '}' ';'
	| '{' fallbackoptionbracket '}'
;

fallbackoption: INTEGER fallbackinterface { CHECK(envFallbackTimeout($1)); }
	| fallbackinterface
	| INTEGER { CHECK(envFallbackTimeout($1)); } ';'
;

fallbackoptionbracket:
	| continuedhcp fallbackinterface timeout
	| continuedhcp timeout fallbackinterface
	| fallbackinterface continuedhcp timeout
	| fallbackinterface timeout continuedhcp
	| timeout continuedhcp fallbackinterface
	| timeout fallbackinterface continuedhcp
	| continuedhcp timeout
	| continuedhcp fallbackinterface
	| fallbackinterface continuedhcp
	| timeout continuedhcp
	| timeout fallbackinterface
	| fallbackinterface timeout
	| fallbackinterface
	| timeout
	| continuedhcp
;

fallbackinterface: zeroconf
	| STATIC { CHECK(envStatic());  } staticconfig { CHECK(finishStatic()); }
;

timeout: TIMEOUT INTEGER ';' { CHECK(envFallbackTimeout($2));}
;

continuedhcp: CONTINUEDHCP BOOL ';' { CHECK(envFallbackContinueDhcp($2)); }
;

zeroconf: ZEROCONF { CHECK(envZeroconf()); } ';' { CHECK(finishZeroconf()); }
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

staticoption_netmask: NETMASK IPv4_VAL ';' { CHECK(staticNetmask(*$2)); delete $2; }
;

staticoption_gateway: GATEWAY IPv4_VAL ';' { CHECK(staticGateway(*$2)); delete $2; }
;

staticoption_dns: DNSSERVER IPv4_VAL ';' { CHECK(staticDNS(*$2)); delete $2; }
;

select: selectfilter
;

selectblock: '{' selectfilters '}'
	| '{' selectfilters '}' ';'
;

selectfilters:
	| selectfilters selectfilter
;

selectfilter: selectfilter2
	| SELECT selectfilter2
;

selectfilter2: sf_user
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

sf_block: AND { CHECK(selectAndBlock()); } selectblock { CHECK(selectBlockEnd()); }
	| OR { CHECK(selectOrBlock()); } selectblock { CHECK(selectBlockEnd()); }
	| { CHECK(selectAndBlock()); } selectblock { CHECK(selectBlockEnd()); }
;

%%

void configparsererror (ConfigParser *cp, const char* msg) {
	cp->parseError(line_num, QString::fromUtf8(msg));
}
