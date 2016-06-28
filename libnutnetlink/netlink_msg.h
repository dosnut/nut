#ifndef _NUT_NETLINK_MSG_H
#define _NUT_NETLINK_MSG_H

#pragma once

#include "netlink.h"

namespace netlink {
	class nl_msg_ref : public internal::generic_ref<internal::desc_nl_msg> {
	public:
		static nl_msg_ref alloc() noexcept;
	};
}

#endif /* _NUT_NETLINK_MSG_H */
