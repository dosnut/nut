#include "netlink_msg.h"

extern "C" {
#include <netlink/msg.h>
}

namespace netlink {
	nl_msg_ref nl_msg_ref::alloc() noexcept {
		return nl_msg_ref::take_own(::nlmsg_alloc());
	}
}
