#include "netlink_addr.h"

extern "C" {
#include <netlink/addr.h>
}

namespace netlink {
	nl_addr_ref nl_addr_ref::alloc_zero(size_t max_addr_len) noexcept {
		return nl_addr_ref::take_own(::nl_addr_alloc(max_addr_len));
	}

	nl_addr_ref nl_addr_ref::create(int family, void const* buf, size_t size) noexcept {
		return nl_addr_ref::take_own(::nl_addr_build(family, buf, size));
	}

	nl_addr_ref nl_addr_ref::create(QHostAddress const& addr) noexcept {
		switch (addr.protocol()) {
		case QAbstractSocket::IPv4Protocol:
			{
				uint32_t ip = htonl(addr.toIPv4Address());
				return create(AF_INET, &ip, 4);
			}
		case QAbstractSocket::IPv6Protocol:
			return create(AF_INET6, addr.toIPv6Address().c, 16);
		case QAbstractSocket::AnyIPProtocol:
			return alloc_zero(16);
		case QAbstractSocket::UnknownNetworkLayerProtocol:
			break;
		}
		return nl_addr_ref();
	}

	nl_addr_ref nl_addr_ref::create(const QPair<QHostAddress, int>& network) noexcept {
		nl_addr_ref res =  create(network.first);
		res.set_prefixlen(network.second);
		return res;
	}

	nl_addr_ref nl_addr_ref::parse(const char* addr, int family_hint, std::error_code& ec) noexcept {
		nl_addr_ref res;
		int err = ::nl_addr_parse(addr, family_hint, res.reset_and_get_ptr_ref());
		if (0 > err) {
			ec = make_netlink_error_code(err);
		}
		return res;
	}

	void nl_addr_ref::set_family(int family) const noexcept {
		::nl_addr_set_family(get(), family);
	}

	int nl_addr_ref::get_family() const noexcept {
		return ::nl_addr_get_family(get());
	}

	void nl_addr_ref::set_binary_addr(const void* buf, size_t size) const noexcept {
		::nl_addr_set_binary_addr(get(), buf, size);
	}

	void* nl_addr_ref::get_binary_addr() const noexcept {
		return ::nl_addr_get_binary_addr(get());
	}

	size_t nl_addr_ref::get_len() const noexcept {
		return ::nl_addr_get_len(get());
	}

	void nl_addr_ref::set_prefixlen(unsigned int prefixlen) const noexcept {
		::nl_addr_set_prefixlen(get(), prefixlen);
	}

	unsigned int nl_addr_ref::get_prefixlen() const noexcept {
		return ::nl_addr_get_prefixlen(get());
	}

	QString nl_addr_ref::toString() const {
		char buf[128];
		return QString::fromUtf8(::nl_addr2str(get(), buf, sizeof(buf)));
	}

	bool nl_addr_ref::iszero() const noexcept {
		return ::nl_addr_iszero(get());
	}

	int nl_addr_ref::guess_family() const noexcept {
		return ::nl_addr_guess_family(get());
	}

	int cmp(const nl_addr_ref& a, const nl_addr_ref& b) noexcept {
		return ::nl_addr_cmp(a.get(), b.get());
	}

	int cmp_prefix(const nl_addr_ref& a, const nl_addr_ref& b) noexcept {
		return ::nl_addr_cmp(a.get(), b.get());
	}
}
