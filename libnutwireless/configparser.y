%{
	#include "cconfigparser.h"
	#include "wpa_supplicant.h"

	#include <QHostAddress>

	extern int configparserlex (void);
	extern int line_num;

	using namespace libnutwireless;

	void configparsererror (libnutwireless::CConfigParser *cp, const char* s);

	//Check if invoked action worked (e.g. newDevice -> device creation worked)
	#define CHECK(action) do { if (!cp->getCurrentNetwork() || !(cp->getCurrentNetwork()->action)) { printf("error\n"); YYERROR;} } while (0)
%}

%union {
	QString *str;
	QHostAddress *addr;
	libnutcommon::MacAddress *macAddr;
	int i;
	bool b;
}

%token NETWORK
%token SSID
%token BSSID
%token DISABLED
%token ID_STR
%token SCAN_SSID
%token PRIORITY
%token MODE
%token FREQUENCY
%token PROTO
%token KEY_MGMT
%token AUTH_ALG
%token PAIRWISE
%token GROUP
%token PSK
%token EAPOL_FLAGS
%token MIXED_CELL
%token PROACTIVE_KEY_CACHING
%token WEP_KEY0
%token WEP_KEY1
%token WEP_KEY2
%token WEP_KEY3
%token WEP_TX_KEYIDX
%token PEERKEY
%token EAP
%token IDENTITY
%token ANONYMOUS_IDENTITY
%token PASSWORD
%token CA_CERT
%token CA_PATH
%token CLIENT_CERT
%token PRIVATE_KEY
%token PRIVATE_KEY_PASSWD
%token DH_FILE
%token SUBJECT_MATCH
%token ALTSUBJECT_MATCH
%token PHASE1
%token PHASE2
%token CA_CERT2
%token CA_PATH2
%token CLIENT_CERT2
%token PRIVATE_KEY2
%token PRIVATE_KEY2_PASSWD
%token DH_FILE2
%token SUBJECT_MATCH2
%token ALTSUBJECT_MATCH2
%token FRAGMENT_SIZE
%token EAPPSK
%token NAI
%token PAC_FILE


%token <str> VALUE
%token <str> STRVAL
%token <macAddr> MACADDR
%token <i> INTEGER
%token <b> BOOL

%error-verbose

%parse-param {libnutwireless::CConfigParser *cp};
%start input

%%

input:
	| input '\n'
	| input NETWORK '{' {if (!cp->newNetwork()) YYERROR;} network '}' {if (!cp->finishNetwork()) YYERROR;}
;

/*
networkconfig:
	'{' {printf("hdjihsdkfhsdlfah");} network '}'
	| '{' network '}' ';'
;*/

network: networkoption
	| network networkoption
;

networkoption: '\n'
	| SSID STRVAL {CHECK(set_ssid(libnutcommon::SSID::fromRaw(CWpaSupplicant::parseConfigString(*$2)))); delete $2;}
	| SSID VALUE {CHECK(set_ssid(libnutcommon::SSID::fromRaw(CWpaSupplicant::parseConfigString(*$2)))); delete $2;}
	| BSSID MACADDR {CHECK(set_bssid(*$2)); delete $2;}
	| DISABLED  BOOL {CHECK(set_disabled($2));}
	| ID_STR STRVAL {CHECK(set_id_str(*$2)); delete $2;}
	| SCAN_SSID BOOL {CHECK(set_scan_ssid($2));}
	| PRIORITY INTEGER {CHECK(set_priority($2));}
	| MODE BOOL {CHECK(set_mode($2));}
	| FREQUENCY INTEGER {CHECK(set_frequency($2));}
	| PROTO VALUE {CHECK(set_proto(*$2)); delete $2;}
	| KEY_MGMT VALUE {CHECK(set_key_mgmt(*$2)); delete $2;}
	| AUTH_ALG VALUE {CHECK(set_auth_alg(*$2)); delete $2;}
	| PAIRWISE VALUE {CHECK(set_pairwise(*$2)); delete $2;}
	| GROUP VALUE {CHECK(set_group(*$2)); delete $2;}
	| PSK VALUE {CHECK(set_psk(*$2)); delete $2;}
	| EAPOL_FLAGS INTEGER {CHECK(set_eapol_flags($2));}
	| MIXED_CELL BOOL {CHECK(set_mixed_cell($2));}
	| PROACTIVE_KEY_CACHING BOOL {CHECK(set_proactive_key_caching($2));}
	| WEP_KEY0 STRVAL {CHECK(set_wep_key0(*$2)); delete $2; }
	| WEP_KEY1 STRVAL {CHECK(set_wep_key1(*$2)); delete $2; }
	| WEP_KEY2 STRVAL {CHECK(set_wep_key2(*$2)); delete $2; }
	| WEP_KEY3 STRVAL {CHECK(set_wep_key3(*$2)); delete $2; }
	| WEP_KEY0 VALUE {CHECK(set_wep_key0(*$2)); delete $2; }
	| WEP_KEY1 VALUE {CHECK(set_wep_key1(*$2)); delete $2; }
	| WEP_KEY2 VALUE {CHECK(set_wep_key2(*$2)); delete $2; }
	| WEP_KEY3 VALUE {CHECK(set_wep_key3(*$2)); delete $2; }
	| WEP_TX_KEYIDX INTEGER {CHECK(set_wep_tx_keyidx($2));}
	| PEERKEY BOOL {CHECK(set_peerkey($2));}
	| EAP VALUE {CHECK(set_eap(*$2)); delete $2;}
	| IDENTITY STRVAL {CHECK(set_identity(*$2)); delete $2;}
	| ANONYMOUS_IDENTITY STRVAL {CHECK(set_anonymous_identity(*$2)); delete $2;}
	| PASSWORD STRVAL {CHECK(set_password(*$2)); delete $2;}
	| CA_CERT STRVAL {CHECK(set_ca_cert(*$2)); delete $2;}
	| CA_PATH STRVAL {CHECK(set_ca_path(*$2)); delete $2;}
	| CLIENT_CERT STRVAL {CHECK(set_client_cert(*$2)); delete $2;}
	| PRIVATE_KEY STRVAL {CHECK(set_private_key(*$2)); delete $2;}
	| PRIVATE_KEY_PASSWD STRVAL {CHECK(set_private_key_passwd(*$2)); delete $2;}
	| DH_FILE STRVAL {CHECK(set_dh_file(*$2)); delete $2;}
	| SUBJECT_MATCH STRVAL {CHECK(set_subject_match(*$2)); delete $2;}
	| ALTSUBJECT_MATCH STRVAL {CHECK(set_altsubject_match(*$2)); delete $2;}
	| PHASE1 STRVAL {CHECK(set_phase1(*$2)); delete $2;}
	| PHASE2 STRVAL {CHECK(set_phase2(*$2)); delete $2;}
	| CA_CERT2 STRVAL {CHECK(set_ca_cert2(*$2)); delete $2;}
	| CA_PATH2 STRVAL {CHECK(set_ca_path2(*$2)); delete $2;}
	| CLIENT_CERT2 STRVAL {CHECK(set_client_cert2(*$2)); delete $2;}
	| PRIVATE_KEY2 STRVAL {CHECK(set_private_key2(*$2)); delete $2;}
	| PRIVATE_KEY2_PASSWD STRVAL {CHECK(set_private_key2(*$2)); delete $2;}
	| DH_FILE2 STRVAL {CHECK(set_dh_file2(*$2)); delete $2;}
	| SUBJECT_MATCH2 STRVAL {CHECK(set_subject_match2(*$2)); delete $2;}
	| ALTSUBJECT_MATCH2 STRVAL {CHECK(set_altsubject_match2(*$2)); delete $2;}
	| FRAGMENT_SIZE INTEGER {CHECK(set_fragment_size($2));}
	| EAPPSK VALUE {CHECK(set_eappsk(*$2)); delete $2;}
	| NAI STRVAL {CHECK(set_nai(*$2)); delete $2;}
	| PAC_FILE STRVAL {CHECK(set_pac_file(*$2)); delete $2;}
;

%%

void configparsererror (libnutwireless::CConfigParser *cp, const char* msg) {
	cp->parseError(line_num, QString::fromUtf8(msg));
}
