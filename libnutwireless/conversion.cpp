#include "conversion.h"

namespace libnutwireless {
int frequencyToChannel(int freq) {
	switch (freq) {
		//2,4GHz part 
		case 2412: return 1;
		case 2417: return 2;
		case 2422: return 3;
		case 2427: return 4;
		case 2432: return 5;
		case 2437: return 6;
		case 2442: return 7;
		case 2447: return 8;
		case 2452: return 9;
		case 2457: return 10;
		case 2462: return 11;
		case 2467: return 12;
		case 2472: return 13;
		case 2484: return 14;
		//5 GHz part
		case 5180: return 36;
		case 5200: return 40;
		case 5220: return 44;
		case 5240: return 48;
		case 5260: return 52;
		case 5280: return 56;
		case 5300: return 60;
		case 5320: return 64;
		case 5500: return 100;
		case 5520: return 104;
		case 5540: return 108;
		case 5560: return 112;
		case 5580: return 116;
		case 5600: return 120;
		case 5620: return 124;
		case 5640: return 128;
		case 5660: return 132;
		case 5680: return 136;
		case 5700: return 140;
		case 5735: return 147;
		case 5755: return 151;
		case 5775: return 155;
		case 5835: return 167;
		default: return -1;
	}
}
int channelToFrequency(int channel) {
	switch(channel) {
		case (1): return 2412;
		case (2): return 2417;
		case (3): return 2422;
		case (4): return 2427;
		case (5): return 2432;
		case (6): return 2437;
		case (7): return 2442;
		case (8): return 2447;
		case (9): return 2452;
		case (10): return 2457;
		case (11): return 2462;
		case (12): return 2467;
		case (13): return 2472;
		case (14): return 2484;
		//(5) GHz part
		case (36): return 5180;
		case (40): return 5200;
		case (44): return 5220;
		case (48): return 5240;
		case (52): return 5260;
		case (56): return 5280;
		case (60): return 5300;
		case (64): return 5320;
		case (100): return 5500;
		case (104): return 5520;
		case (108): return 5540;
		case (112): return 5560;
		case (116): return 5580;
		case (120): return 5600;
		case (124): return 5620;
		case (128): return 5640;
		case (132): return 5660;
		case (136): return 5680;
		case (140): return 5700;
		case (147): return 5735;
		case (151): return 5755;
		case (155): return 5775;
		case (167): return 5835;
		default: return -1;
	}
}

}