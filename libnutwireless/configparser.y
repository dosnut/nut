%{
	#include "cconfigparser.h"
	

	extern int configparserlex (void);
	extern int line_num;
	
	using namespace libnutwireless;
	
	static void configparsererror (ConfigParser *cp, const char* s);
	
	//Check if invoked action worked (e.g. newDevice -> device creation worked)
	#define CHECK(action) do { if (!cp->currentNetwork || !(cp->currentNetwork->action)) YYERROR; } while (0)
%}

%union {
	QString *str;
	QHostAddress *addr;
	libnutcommon::MacAddress *macAddr;
	int i;
	bool b;
}

%token NETWORK


%token <str> STRING
%token <macAddr> MACADDR
%token <i> INTEGER
%token <b> BOOL

%error-verbose

%parse-param {CConfigParser *cp};
%start input

%%

input:
	| input NETWORK {if (!newNetwork()) YYERROR;} networkconfig {if (!finishNetwork()) YYERROR;}
;

networkconfig:
	| '{' network '}'
	| '{' network '}' ';'
;

network:
	| networkoptions networkoption
;
networkoption:
	| SSID '=' STRING {CHECK(set_ssid(*$3)); delete $3;}
	| BSSID '=' MACADDR {CHECK(set_bssid(*$3)); delete $3;}
	| DISABLED '=' BOOL {CHECK(set_disabled($3));}
	| ID_STR '=' STRING {CHECK(set_id_str(*$3)); delete $3;}
	| SCAN_SSID '=' BOOL {CHECK(set_scan_ssid($3));}
	| PRIORITY '=' INTEGER {CHECK(set_priority($3));}
	| MODE '=' BOOL {CHECK(set_mode($3));}
	| FREQUENCY '=' INTEGER {CHECK(set_frequency($3));}
	| PROTO '=' STRING {CHECK(set_proto(*$3)); delete $3;}
	| KEY_MGMT '=' STRING {CHECK(set_key_mgmt(*$3)); delete $3;}
	| AUTH_ALG '=' STRING {CHECK(set_auth_alg(*$3)); delete $3;}
	| PAIRWISE '=' STRING {CHECK(set_pairwise(*$3)); delete $3;}
	| GROUP '=' STRING {CHECK(set_group(*$3)); delete $3;}
	| PSK '=' STRING {CHECK(set_psk(*$3)); delete $3;}
	| EAPOL_FLAGS '=' STRING {CHECK(set_eapol_flags(*$3)); delete $3;}
	| MIXED_CELL '=' BOOL {CHECK(set_mixed_cell($3));}
	| PROACTIVE_KEY_CACHING '=' BOOL {CHECK(set_proactive_key_caching($3));}
	| WEP_KEY0 '=' STRING {CHECK(set_wep_key0(*$3, false); delete $3; }
	| WEP_KEY1 '=' STRING {CHECK(set_wep_key1(*$3, false); delete $3; }
	| WEP_KEY2 '=' STRING {CHECK(set_wep_key2(*$3, false); delete $3; }
	| WEP_KEY3 '=' STRING {CHECK(set_wep_key3(*$3, false); delete $3; }
	| WEP_TX_KEYIDX '=' INTEGER {CHECK(set_wep_tx_keyidx($3));}
	| PEERKEY '=' BOOL {CHECK(set_peerkey($3));}
	| EAP '=' STRING {CHECK(set_eap(*$3)); delete $3;}
	| IDENTITY '=' STRING {CHECK(set_identity(*$3)); delete $3;}
	| ANONYMOUS_IDENTITY '=' STRING {CHECK(set_anonymous_identity(*$3)); delete $3;}
	| PASSWORD '=' STRING {CHECK(set_password(*$3)); delete $3;}
	| CA_CERT '=' STRING {CHECK(set_ca_cert(*$3)); delete $3;}
	| CA_PATH '=' STRING {CHECK(set_ca_path(*$3)); delete $3;}
	| CLIENT_CERT '=' STRING {CHECK(set_client_cert(*$3)); delete $3;}
	| PRIVATE_KEY '=' STRING {CHECK(set_private_key(*$3)); delete $3;}
	| PRIVATE_KEY_PASSWD '=' STRING {CHECK(set_private_key_passwd(*$3)); delete $3;}
	| DH_FILE '=' STRING {CHECK(set_dh_file(*$3)); delete $3;}
	| SUBJECT_MATCH '=' STRING {CHECK(set_subject_match(*$3)); delete $3;}
	| ALTSUBJECT_MATCH '=' STRING {CHECK(set_altsubject_match(*$3)); delete $3;}
	| PHASE1 '=' STRING {CHECK(set_phase1(*$3)); delete $3;}
	| PHASE2 '=' STRING {CHECK(set_phase2(*$3)); delete $3;}
	| CA_CERT2 '=' STRING {CHECK(set_ca_cert2(*$3)); delete $3;}
	| CA_PATH2 '=' STRING {CHECK(set_ca_path2(*$3)); delete $3;}
	| CLIENT_CERT2 '=' STRING {CHECK(set_client_cert2(*$3)); delete $3;}
	| PRIVATE_KEY2 '=' STRING {CHECK(set_private_key2(*$3)); delete $3;}
	| PRIVATE_KEY2_PASSWD '=' STRING {CHECK(set_private_key2(*$3)); delete $3;}
	| DH_FILE2 '=' STRING {CHECK(set_dh_file2(*$3)); delete $3;}
	| SUBJECT_MATCH2 '=' STRING {CHECK(set_subject_match2(*$3)); delete $3;}
	| ALTSUBJECT_MATCH2 '=' STRING {CHECK(set_altsubject_match2(*$3)); delete $3;}
	| FRAGMENT_SIZE '=' INTEGER {CHECK(set_fragment_size($3));}
	| EAPPSK '=' STRING {CHECK(set_eappsk(*$3)); delete $3;}
	| NAI '=' STRING {CHECK(set_nai(*$3)); delete $3;}
	| PAC_FILE '=' STRING {CHECK(set_pac_file(*$3)); delete $3;}
;

%%

void configparsererror (ConfigParser *cp, const char* msg) {
	cp->parseError(line_num, QString::fromUtf8(msg));
}
