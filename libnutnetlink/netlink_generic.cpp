#include "netlink_generic.h"

#include "netlink_msg.h"

extern "C" {
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/mngt.h>
}

namespace netlink {
	namespace {
		template<typename Integral>
		boost::optional<Integral> read_attr_integral(::nlattr* attr, std::error_code& ec) noexcept {
			size_t len = static_cast<size_t>(::nla_len(attr));
			Integral result{0};
			if (len < sizeof(result)) {
				ec = errc::range;
				return boost::none;
			} else {
				memcpy(&result, ::nla_data(attr), sizeof(result));
				return result;
			}
		}

		boost::optional<QString> read_attr_string(::nlattr* attr, std::error_code& ec) noexcept {
			size_t len = static_cast<size_t>(::nla_len(attr));
			if (len < 1) {
				ec = errc::range;
			} else {
				char const* const data = reinterpret_cast<char const*>(::nla_data(attr));
				if ('\0' != data[len - 1]) {
					ec = errc::inval;
				} else {
					return QString::fromUtf8(data, len - 1);
				}
			}
			return boost::none;
		}
	}

	genl_group::genl_group(QString name, uint32_t id) noexcept
		: m_name(name), m_id(id) {
	}

	QString genl_group::get_name() const {
		return m_name;
	}

	uint32_t genl_group::get_id() const {
		return m_id;
	}

	static boost::optional<genl_group> parse_group(::nlattr* container, std::error_code& ec) {
		::nlattr* attr = reinterpret_cast<::nlattr*>(::nla_data(container));

		boost::optional<QString> a_name;
		boost::optional<uint32_t> a_id;

		for (int rem = ::nla_len(container); ::nla_ok(attr, rem); attr = ::nla_next(attr, &rem)) {
			switch (attr->nla_type) {
			case CTRL_ATTR_MCAST_GRP_NAME:
				if (!(a_name = read_attr_string(attr, ec))) return boost::none;
				break;
			case CTRL_ATTR_MCAST_GRP_ID:
				if (!(a_id = read_attr_integral<uint32_t>(attr, ec))) return boost::none;
				break;
			}
		}

		if (!a_name || !a_id) {
			ec = errc::missing_attr;
			return boost::none;
		}
		return genl_group(*a_name, *a_id);
	}

	static boost::optional<std::vector<genl_group>> parse_nested_groups(::nlattr* container, std::error_code& ec) noexcept {
		std::vector<genl_group> groups;
		::nlattr* attr = reinterpret_cast<::nlattr*>(::nla_data(container));
		for (int rem = ::nla_len(container); ::nla_ok(attr, rem); attr = ::nla_next(attr, &rem)) {
			if (auto elem = parse_group(attr, ec)) {
				groups.push_back(*elem);
			} else {
				return boost::none;
			}
		}
		return std::move(groups);
	}

	genl_op::genl_op(uint32_t op, uint32_t flags) noexcept
	: m_op(op), m_flags(flags) {
	}

	uint32_t genl_op::get_op() const noexcept {
		return m_op;
	}

	uint32_t genl_op::get_flags() const noexcept {
		return m_flags;
	}

	static boost::optional<genl_op> parse_op(::nlattr* container, std::error_code& ec) {
		::nlattr* attr = reinterpret_cast<::nlattr*>(::nla_data(container));

		boost::optional<uint32_t> a_id;
		boost::optional<uint32_t> a_flags;

		for (int rem = ::nla_len(container); ::nla_ok(attr, rem); attr = ::nla_next(attr, &rem)) {
			switch (attr->nla_type) {
			case CTRL_ATTR_OP_ID:
				if (!(a_id = read_attr_integral<uint32_t>(attr, ec))) return boost::none;
				break;
			case CTRL_ATTR_OP_FLAGS:
				if (!(a_flags = read_attr_integral<uint32_t>(attr, ec))) return boost::none;
				break;
			}
		}

		if (!a_id) {
			ec = errc::missing_attr;
			return boost::none;
		}
		return genl_op(*a_id, a_flags.value_or(0));
	}

	static boost::optional<std::vector<genl_op>> parse_nested_ops(::nlattr* container, std::error_code& ec) noexcept {
		std::vector<genl_op> groups;
		::nlattr* attr = reinterpret_cast<::nlattr*>(::nla_data(container));
		for (int rem = ::nla_len(container); ::nla_ok(attr, rem); attr = ::nla_next(attr, &rem)) {
			if (auto elem = parse_op(attr, ec)) {
				groups.push_back(*elem);
			} else {
				return boost::none;
			}
		}
		return std::move(groups);
	}

	genl_family genl_family::parse(::nl_msg* msg, std::error_code& ec) noexcept {
		::nlmsghdr* nlh = ::nlmsg_hdr(msg);
		int const hdrlen = 0;

		if (!::genlmsg_valid_hdr(nlh, hdrlen)) {
			ec = errc::msg_tooshort;
			return genl_family{};
		}

		::genlmsghdr* ghdr = reinterpret_cast<::genlmsghdr*>(::nlmsg_data(nlh));
		::nlattr* attr = ::genlmsg_attrdata(ghdr, 0);
		int attr_len = ::genlmsg_attrlen(ghdr, hdrlen);

		boost::optional<QString> a_name;
		boost::optional<uint16_t> a_id;
		boost::optional<uint32_t> a_version;
		boost::optional<uint32_t> a_hdrsize;
		boost::optional<uint32_t> a_maxattr;
		boost::optional<std::vector<genl_op>> a_ops;
		boost::optional<std::vector<genl_group>> a_groups;

		for (int rem = attr_len; ::nla_ok(attr, rem); attr = ::nla_next(attr, &rem)) {
			switch (attr->nla_type) {
			case CTRL_ATTR_FAMILY_NAME:
				if (!(a_name = read_attr_string(attr, ec))) return genl_family{};
				break;
			case CTRL_ATTR_FAMILY_ID:
				if (!(a_id = read_attr_integral<uint16_t>(attr, ec))) return genl_family{};
				break;
			case CTRL_ATTR_VERSION:
				if (!(a_version = read_attr_integral<uint32_t>(attr, ec))) return genl_family{};
				break;
			case CTRL_ATTR_HDRSIZE:
				if (!(a_hdrsize = read_attr_integral<uint32_t>(attr, ec))) return genl_family{};
				break;
			case CTRL_ATTR_MAXATTR:
				if (!(a_maxattr = read_attr_integral<uint32_t>(attr, ec))) return genl_family{};
				break;
			case CTRL_ATTR_OPS:
				if (!(a_ops = parse_nested_ops(attr, ec))) return genl_family{};
				break;
			case CTRL_ATTR_MCAST_GROUPS:
				if (!(a_groups = parse_nested_groups(attr, ec))) return genl_family{};
				break;
			}
		}

		if (!a_name || !a_id) {
			ec = errc::missing_attr;
			return genl_family{};
		}

		genl_family result{};
		result.m_name = *a_name;
		result.m_id = *a_id;
		result.m_version = a_version;
		result.m_hdrsize = a_hdrsize;
		result.m_maxatttr = a_maxattr;
		if (a_ops) result.m_ops = std::move(*a_ops);
		if (a_groups) result.m_groups= std::move(*a_groups);
		return result;
	}

	QString genl_family::get_name() const noexcept {
		return m_name;
	}

	uint16_t genl_family::get_id() const noexcept {
		return m_id;
	}

	boost::optional<uint32_t> genl_family::get_version() const noexcept {
		return m_version;
	}

	boost::optional<uint32_t> genl_family::get_hdrsize() const noexcept {
		return m_hdrsize;
	}

	boost::optional<uint32_t> genl_family::get_maxatttr() const noexcept {
		return m_maxatttr;
	}

	std::vector<genl_op> const& genl_family::get_ops() const noexcept {
		return m_ops;
	}

	std::vector<genl_group> const& genl_family::get_groups() const noexcept {
		return m_groups;
	}

	generic_netlink_sock generic_netlink_sock::connect(char const* name, std::error_code& ec) noexcept {
		generic_netlink_sock result;
		result.m_sock = nl_socket_ptr::alloc();
		if ((ec = result.m_sock.connect(protocol::generic))) return generic_netlink_sock{};

		// genl_ctrl_probe_by_name
		auto msg = nl_msg_ref::alloc();
		if (!::genlmsg_put(
					msg, NL_AUTO_PORT, NL_AUTO_SEQ, GENL_ID_CTRL,
					0, 0, CTRL_CMD_GETFAMILY, 1)) {
			ec = errc::nomem;
			return generic_netlink_sock{};
		}

		if (int err = ::nla_put_string(msg, CTRL_ATTR_FAMILY_NAME, name)) {
			ec = make_netlink_error_code(err);
			return generic_netlink_sock{};
		}

		if ((ec = result.m_sock.send_auto(msg))) return generic_netlink_sock{};

		auto cb = result.m_sock.get_cb().clone();
		std::error_code parse_ec{errc::failure};
		cb.set(callback_type::valid, [&result, &parse_ec](::nl_msg* msg) -> int {
			parse_ec = std::error_code{};
			result.m_family = genl_family::parse(msg, parse_ec);
			return NL_OK;
		});

		if ((ec = result.m_sock.recvmsgs(cb))) return generic_netlink_sock{};
		if (parse_ec) {
			ec = parse_ec;
			return generic_netlink_sock{};
		}
		if ((ec = result.m_sock.wait_for_ack())) return generic_netlink_sock{};

		return result;
	}

	generic_netlink_sock generic_netlink_sock::connect(char const* name) {
		std::error_code ec;
		auto result = connect(name, ec);
		if (ec) throw ec;
		return result;
	}

	generic_netlink_sock generic_netlink_sock::connect_groups(char const* name, std::initializer_list<char const*> groups, std::error_code& ec) noexcept {
		auto result = connect(name, ec);
		if (ec) return generic_netlink_sock{};
		for (char const* group: groups) {
			if ((ec = result.add_membership(group))) return generic_netlink_sock{};
		}
		return result;
	}

	generic_netlink_sock generic_netlink_sock::connect_groups(char const* name, std::initializer_list<char const*> groups) {
		std::error_code ec;
		auto result = connect_groups(name, groups, ec);
		if (ec) throw ec;
		return result;
	}

	void generic_netlink_sock::reset() noexcept {
		m_sock.reset();
		m_family = genl_family{};
	}

	std::error_code generic_netlink_sock::add_membership(const char* group) const noexcept {
		for (genl_group const &grp: m_family.get_groups()) {
			if (grp.get_name() == group) {
				return m_sock.add_membership_raw(grp.get_id());
			}
		}
		return errc::obj_notfound;
	}

	std::error_code generic_netlink_sock::drop_membership(const char* group) const noexcept {
		for (genl_group const &grp: m_family.get_groups()) {
			if (grp.get_name() == group) {
				return m_sock.drop_membership_raw(grp.get_id());
			}
		}
		return errc::obj_notfound;
	}

	nl_socket_ptr const& generic_netlink_sock::get_socket() const noexcept {
		return m_sock;
	}

	genl_family const& generic_netlink_sock::get_family() const noexcept {
		return m_family;
	}


#if 0
	genl_family_ref nl_socket_ptr::family_by_name(const char* name, std::error_code& ec) const noexcept {
		auto familyCache = genl_family_cache_ref::alloc();
		if (auto lec = refill(familyCache)) {
			ec = lec;
			return genl_family_ref{};
		}
		return genl_family_ref::take_own(::genl_ctrl_search_by_name(familyCache.get(), name));
	}

	genl_family_ref nl_socket_ptr::family_by_name(const char* name) const {
		std::error_code ec;
		auto result = family_by_name(name, ec);
		if (ec) throw ec;
		return result;
	}

	std::error_code nl_socket_ptr::add_memberships(genl_family_ref const& family, std::initializer_list<char const*> groups) const noexcept {
		for (char const* group: groups) {

		}
	}

	std::error_code nl_socket_ptr::drop_memberships(genl_family_ref const& family, std::initializer_list<char const*> groups) const noexcept {

	}
#endif

}
