
%x String
%x Comment
%option bison-bridge bison-locations reentrant
%option stack
%option noyywrap
%option nounput
%option noyy_top_state

%{
	#include "configparser_p.h"

	using namespace nuts;

	#define YYSTYPE config_tokenvalue
	#define YYLTYPE config_lloc
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



(true|TRUE|True) {
            yylval->b = true; return BOOL; }

(false|FALSE|False) {
            yylval->b = false; return BOOL; }

([[:digit:]]{1,3}\.){3}[[:digit:]]{1,3} {
            yylval->addr = QHostAddress(QString(yytext)); return IPv4_VAL; }

([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2} {
            yylval->macAddr = libnutcommon::MacAddress(QString(yytext)); return MACADDR; }

-?[[:digit:]]+ {
            yylval->i = atoi(yytext); return INTEGER; }

\"          yylval->str.clear(); BEGIN(String);
"/*"        yy_push_state(Comment, yyscanner);

\n          ++yylloc->lineNum;
"//".*\n    ++yylloc->lineNum;
[!{};/]     return yytext[0];
[ \t\v\r]+  /* ignore white space */
[a-zA-Z0-9_.'\-+*#]+  {
            yylval->str = QString(yytext); return INVALID_TOKEN; }
.           {
            yylval->str = QString(yytext); return INVALID_CHAR; }

<String>{
	\"          { BEGIN(INITIAL); return STRING; }
	[^\\\"\n]*  { yylval->str.append(yytext); }
	\n          { printf("Unexpected newline in string, ending string\n");
	              ++yylloc->lineNum;
	              BEGIN(INITIAL); return STRING;
	            }
	\\n         { yylval->str.append("\n"); }
	\\\"        { yylval->str.append("\""); }
	\\.         { yylval->str.append(yytext[1]); }
}

	/* comments may be nested */
<Comment>{
	[^/*\n]*        /* eat anything that's not a '*', '/' or newline */
	"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
	"*"+"/"        yy_pop_state(yyscanner);
	"/"+[^*/\n]*
	"/"+"*"        yy_push_state(Comment, yyscanner);
	\n             ++yylloc->lineNum;
}

%%
