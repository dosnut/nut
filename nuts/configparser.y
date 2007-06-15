%{
	#include "config.h"

	extern int configparserlex (void);
	extern int line_num;
	
	using namespace nuts;
	
	static void configparsererror (Config *config, char* s);

	static DeviceConfig *curdevconfig;
	static EnvironmentConfig *curenvconfig;
	static IPv4Config *curipv4config;
%}

%union {
	QString *str;
	QHostAddress *addr;
	int i;
}

%token DEVICE ENVIRONMENT
%token DEFAULT
%token DHCP NODHCP ZEROCONF NOZEROCONF STATIC
%token IP NETMASK GATEWAY DNSSERVER
%token LABELINDEX
%token SELECT USER ARP ESSID
%token WLAN MODE WPACONFIG

%token <str> STRING
%token <addr> IPv4_VAL
%token <i> INTEGER
%error-verbose

%parse-param {Config *curconfig};
%start input

%%

input:
	| input device
;

device: DEVICE STRING { curdevconfig = curconfig->createDevice(*$2); } deviceconfig ';'
;

deviceconfig:
	| '{' deviceoptions '}'
	| deviceoption
;

deviceoptions:
	| deviceoptions deviceoption
;

deviceoption: { curenvconfig = curdevconfig->getDefaultEnv(); } environmentoption
	| environment
;

environment: ENVIRONMENT { curenvconfig = curdevconfig->createEnvironment(); } environmentconfig ';'
;

environmentconfig:
	| '{' environmentoptions '}'
	| environmentoption
;

environmentoptions:
	| environmentoptions environmentoption
;

environmentoption: dhcpconfig
;

dhcpconfig: DHCP ';' { curenvconfig->doDHCP(); }
;

%%

void configparsererror (Config *, char*) {
}
