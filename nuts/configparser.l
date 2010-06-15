
%x String
%x Comment
%option stack
%option noyywrap
%option nounput
%option noyy_top_state

%{
	#include "config.h"
	int line_num = 1;

	#ifdef USED_CMAKE	
	#include "configparser_yacc.hpp"
	#else
	#include "configparser_yacc.h"
	#endif
	#define yylval configparserlval
%}

%%

device      return DEVICE;
regexp      return REGEXP;
environment return ENVIRONMENT;

no-auto-start return NOAUTOSTART;

default     return DEFAULT;

dhcp        return DHCP;
no-dhcp     return NODHCP;
zeroconf    return ZEROCONF;
no-zeroconf return NOZEROCONF;
static      return STATIC;
timeout     return TIMEOUT;
fallback    return FALLBACK;
continue-dhcp return CONTINUEDHCP;

ip          return IP;
netmask     return NETMASK;
gateway     return GATEWAY;
dns-server  return DNSSERVER;
metric      return METRIC;

label-index return LABELINDEX;

select      return SELECT;
user        return USER;
arp         return ARP;
essid       return ESSID;
and         return AND;
or          return OR;

wlan        return WLAN;
mode        return MODE;

wpa-supplicant  return WPASUPPLICANT;
config      return CONFIG;
driver      return DRIVER;



(1|true|TRUE|True) {
			yylval.b = true; return BOOL; }

(0|false|FALSE|False) {
			yylval.b = false; return BOOL; }

([[:digit:]]{1,3}\.){3}[[:digit:]]{1,3} {
            yylval.addr = new QHostAddress(QString(yytext)); return IPv4_VAL; }

([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2} {
            yylval.macAddr = new libnutcommon::MacAddress(QString(yytext)); return MACADDR; }

(-[[:blank:]]*)?[[:digit:]]+ {
            yylval.i = atoi(yytext); return INTEGER; }

\"         yylval.str = new QString(); BEGIN(String);
"/*"       yy_push_state(Comment);

\n         ++line_num;
"//".*\n   ++line_num;
[a-zA-Z0-9]+  printf("Unexpected keyword, ignoring\n");
[!{};/]     return yytext[0];
.          /* ignore. TODO: only ignore whitespaces*/

<String>{
	\"          { BEGIN(INITIAL); return STRING; }	
	[^\\\"\n]*  { yylval.str->append(yytext); }
	\n          { printf("Unexpected newline in string, ending string\n");
	              ++line_num;
	              BEGIN(INITIAL); return STRING;
	            }
	\\n         { yylval.str->append("\n"); }
	\\\"        { yylval.str->append("\""); }
	\\.         { yylval.str->append(yytext[1]); }
}

	/* comments may be nested */
<Comment>{
	[^/*\n]*        /* eat anything that's not a '*', '/' or newline */
	"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
	"*"+"/"        yy_pop_state();
	"/"+[^*/\n]*
	"/"+"*"        yy_push_state(Comment);
	\n             ++line_num;
}

%%
