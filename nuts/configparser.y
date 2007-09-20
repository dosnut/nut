%{
	#include "config.h"
	

	extern int configparserlex (void);
	extern int line_num;
	
	using namespace nuts;
	
	static void configparsererror (ConfigParser *cp, char* s);
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
%error-verbose

%parse-param {ConfigParser *cp};
%start input

%%

input:
	| input device
;

device: DEVICE STRING { if (!cp->newDevice(*$2)) YYERROR; } deviceconfig
;

deviceconfig: ';'
	| '{' deviceoptions '}'
	| '{' deviceoptions '}' ';'
	| deviceoption
;

deviceoptions:
	| deviceoptions deviceoption
;

deviceoption: { if (!cp->devDefaultEnvironment()) YYERROR; } environmentoption
	| environment
	| wpasupplicant
	| NOAUTOSTART { if (!cp->devNoAutoStart()) YYERROR; } ';'
;

wpasupplicant: WPASUPPLICANT DRIVER STRING CONFIG STRING ';' { if (!cp->devWPASuppConfig(*$3, *$5)) YYERROR; }
	| WPASUPPLICANT CONFIG STRING ';' { if (!cp->devWPASuppConfig("wext", *$3)) YYERROR; }
	| WPASUPPLICANT CONFIG STRING DRIVER STRING ';' { if (!cp->devWPASuppConfig(*$5, *$3)) YYERROR; }
;

environment: ENVIRONMENT STRING { cp->devEnvironment(*$2); } environmentconfig
	| ENVIRONMENT { if (!cp->devEnvironment("")) YYERROR; } environmentconfig
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
	| { if (!cp->envSelect()) YYERROR; } select
;

dhcpconfig: DHCP ';' { if (!cp->envDHCP()) YYERROR; }
;

static: STATIC { if (!cp->envStatic()) YYERROR;  } staticconfig
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

staticoption_ip: IP IPv4_VAL ';' { if (!cp->staticIP(*$2)) YYERROR; }
;

staticoption_netmask: NETMASK IPv4_VAL ';' { if (!cp->staticNetmak(*$2)) YYERROR; }
;

staticoption_gateway: GATEWAY IPv4_VAL ';' { if (!cp->staticGateway(*$2)) YYERROR; }
;

staticoption_dns: DNSSERVER IPv4_VAL ';' { if (!cp->staticDNS(*$2)) YYERROR; }
;

select: selectstart selectblock
;

selectstart: SELECT AND { if (!cp->selectAndBlock()) YYERROR; }
	| SELECT OR { if (!cp->selectOrBlock()) YYERROR; }
	| SELECT { if (!cp->selectAndBlock()) YYERROR; }
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

sf_user: USER ';' { if (!cp->selectUser()) YYERROR; }
;

sf_arp: ARP IPv4_VAL ';' { if (!cp->selectARP(*$2)) YYERROR; }
	| ARP IPv4_VAL MACADDR ';' { if (!cp->selectARP(*$2, *$3)) YYERROR; }
;

sf_essid: ESSID STRING ';' { if (!cp->selectESSID(*$2)) YYERROR; }
;

sf_block: select
;

%%

void configparsererror (ConfigParser *, char*) {
}
