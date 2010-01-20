#include "wstypes.h"
namespace libnutwireless {
	QString toString(RequestType reqt) {
		switch (reqt) {
			case (REQ_IDENTITY):
				return QString("IDENTITY");
				break;
			case (REQ_NEW_PASSWORD):
				return QString("NEW_PASSWORD");
				break;
			case (REQ_PIN):
				return QString("PIN");
				break;
			case (REQ_OTP):
				QString("OTP");
				break;
			case (REQ_PASSPHRASE):
				QString("PASSPHRASE");
				break;
			default:
				return QString();
		}
		return QString();
	}
}