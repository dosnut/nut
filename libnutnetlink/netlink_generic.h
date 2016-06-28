#ifndef _NUT_NETLINK_GENERIC_H
#define _NUT_NETLINK_GENERIC_H

#pragma once

#include "netlink_sock.h"

#include <boost/optional.hpp>

#include <vector>

namespace netlink {
	class genl_group {
	public:
		explicit genl_group() noexcept {
		}
		explicit genl_group(QString name, uint32_t id) noexcept;

		QString get_name() const;
		uint32_t get_id() const;

	private:
		QString m_name;
		uint32_t m_id{0};
	};

	class genl_op {
	public:
		explicit constexpr genl_op() noexcept {
		}
		explicit genl_op(uint32_t op, uint32_t flags) noexcept;

		uint32_t get_op() const noexcept;
		uint32_t get_flags() const noexcept;

	private:
		uint32_t m_op{0};
		uint32_t m_flags{0};
	};

	class genl_family {
	public:
		explicit genl_family() noexcept {
		}

		static genl_family parse(::nl_msg* msg, std::error_code& ec) noexcept;

		QString get_name() const noexcept;
		uint16_t get_id() const noexcept;
		boost::optional<uint32_t> get_version() const noexcept;
		boost::optional<uint32_t> get_hdrsize() const noexcept;
		boost::optional<uint32_t> get_maxatttr() const noexcept;
		std::vector<genl_op> const& get_ops() const noexcept;
		std::vector<genl_group> const& get_groups() const noexcept;

	private:
		QString m_name;
		uint16_t m_id{0};
		boost::optional<uint32_t> m_version;
		boost::optional<uint32_t> m_hdrsize;
		boost::optional<uint32_t> m_maxatttr;
		std::vector<genl_op> m_ops;
		std::vector<genl_group> m_groups;
	};

	class generic_netlink_sock {
	public:
		explicit generic_netlink_sock() noexcept {
		}

		static generic_netlink_sock connect(char const* name, std::error_code& ec) noexcept;
		static generic_netlink_sock connect(char const* name);

		static generic_netlink_sock connect_groups(char const* name, std::initializer_list<char const*> groups, std::error_code& ec) noexcept;
		static generic_netlink_sock connect_groups(char const* name, std::initializer_list<char const*> groups);

		void reset() noexcept;

		std::error_code add_membership(const char* group) const noexcept;
		std::error_code drop_membership(const char* group) const noexcept;

		nl_socket_ptr const& get_socket() const noexcept;
		genl_family const& get_family() const noexcept;

	private:
		nl_socket_ptr m_sock;
		genl_family m_family;
	};
}

#endif /* _NUT_NETLINK_GENERIC_H */
