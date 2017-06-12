#include "wstypes.h"

namespace libnutwireless {
	QString toString(RequestType reqt) {
		switch (reqt) {
		case REQ_FAIL:
			return QString("FAIL");
		case REQ_PASSWORD:
			return QString("PASSWORD");
		case REQ_IDENTITY:
			return QString("IDENTITY");
		case REQ_NEW_PASSWORD:
			return QString("NEW_PASSWORD");
		case REQ_PIN:
			return QString("PIN");
		case REQ_OTP:
			return QString("OTP");
		case REQ_PASSPHRASE:
			return QString("PASSPHRASE");
		}
		return QString();
	}
}
