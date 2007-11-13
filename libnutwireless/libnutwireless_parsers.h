#ifndef LIBNUTWIRELESS_LIBNUTWIRELESS_PARSERS
#define LIBNUTWIRELESS_LIBNUTWIRELESS_PARSERS

#include "libnut_wpa_supplicant_types.h"
#include <QStringList>


namespace libnutwireless {
	class CWpa_SupplicantParsers {
		protected:

		QStringList sliceMessage(QString str);
		
		//Parse MIB Variables
		MIBVariables parseMIB(QStringList list);
		MIBVariable::MIBVariable_type parseMIBType(QString str);
		
		//parse list network
		QList<ShortNetworkInfo> parseListNetwork(QStringList list);
		NetworkFlags parseNetworkFlags(QString str);
	
	
		//parse scan results
		ScanCiphers parseScanCiphers(QString str);
		ScanAuthentication parseScanAuth(QString str);
		QList<ScanResult> parseScanResult(QStringList list);
	
		//parse config
		Protocols parseProtocols(QString str);
		KeyManagement parseKeyMgmt(QString str);
		AuthenticationAlgs parseAuthAlg(QString str);
		PairwiseCiphers parsePairwiseCiphers(QString str);
		GroupCiphers parseGroupCiphers(QString str);
		EapolFlags parseEapolFlags(QString str);
		EapMethod parseEapMethod(QString str);
		
		
	
		//parse Status with helper functionss
		Status parseStatus(QStringList list);
		Status::WPA_STATE parseWpaState(QString str);
		Status::PAE_STATE parsePaeState(QString str);
		Status::PORT_STATUS parsePortStatus(QString str);
		Status::PORT_CONTROL parsePortControl(QString str);
		Status::BACKEND_STATE parseBackendState(QString str);
		Status::EAP_STATE parseEapState(QString str);
		Status::METHOD_STATE parseMethodState(QString str);
		Status::DECISION parseDecision(QString str);
	
		
		//parse Event
		EventType parseEvent(QString str);
		Request parseReq(QString str);
		RequestType parseReqType(QString str);
		InteractiveType parseInteract(QString str);
	
	};
}
#endif

