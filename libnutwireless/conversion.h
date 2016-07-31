#ifndef LIBNUTWIRELESS_CONVERSION_H
#define LIBNUTWIRELESS_CONVERSION_H

#pragma once

#include "types.h"

namespace libnutwireless {
	/** Convert frequency to channel: 2,4GHz => 1-14; 5GHz => 36-167
		@return -1 on invalid values */
	int frequencyToChannel(int freq);

	/** Convert channel to frequency: 1-14 => 2,4GHz; 36-167 => 5GHz
		@return -1 on invalid values */
	int channelToFrequency(int channel);
}
#endif /* LIBNUTWIRELESS_CONVERSION_H */
