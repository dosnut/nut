#include "version.h"

#ifndef NUT_VERSION
# define NUT_VERSION "unknown version (unsupported buildsystem)"
#endif

namespace libnutcommon {
	namespace {
		QString v{ NUT_VERSION };
	}
	QString const& version() { 
		return v;
	}
}
