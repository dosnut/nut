#include "version.h"

#ifndef NUT_VERSION
# define NUT_VERSION "unknown version (unsupported buildsystem)"
#endif

namespace libnutcommon {
	namespace {
		QString const v{ NUT_VERSION };
	}
	QString version() { 
		return v;
	}
}
