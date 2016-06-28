#include "netlink_rtnl_link.h"
#include "netlink_addr.h"

namespace netlink {
	QString toString(link_flags_t link_flags) {
		char buf[1024];
		return QString::fromUtf8(::rtnl_link_flags2str(link_flags, buf, sizeof(buf)));
	}

	QString toString(operstate_t operstate) {
		char buf[32];
		return QString::fromUtf8(::rtnl_link_operstate2str(static_cast<uint8_t>(operstate), buf, sizeof(buf)));
	}


	QString toString(linkmode_t linkmode) {
		char buf[32];
		return QString::fromUtf8(::rtnl_link_mode2str(static_cast<uint8_t>(linkmode), buf, sizeof(buf)));
	}

	QString toString(carrier_t carrier) {
		char buf[32];
		return QString::fromUtf8(::rtnl_link_carrier2str(static_cast<uint8_t>(carrier), buf, sizeof(buf)));
	}

	rtnl_link_ref rtnl_link_ref::alloc() noexcept {
		return rtnl_link_ref::take_own(::rtnl_link_alloc());
	}

	std::error_code rtnl_link_ref::add(nl_socket_ptr const& sock, new_flags flags) const noexcept {
		int err = ::rtnl_link_add(sock.get(), get(), int(flags));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code rtnl_link_ref::change(nl_socket_ptr const& sock, rtnl_link_ref const& orig, rtnl_link_ref const& changes, new_flags flags) noexcept {
		int err = ::rtnl_link_change(sock.get(), orig.get(), changes.get(), int(flags));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code rtnl_link_ref::remove(nl_socket_ptr const& sock) const noexcept {
		int err = ::rtnl_link_delete(sock.get(), get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	rtnl_link_ref rtnl_link_ref::get_link(nl_socket_ptr const& sock, int ifIndex, std::error_code& ec) noexcept {
		rtnl_link_ref res;
		int err = rtnl_link_get_kernel(sock.get(), ifIndex, nullptr, res.reset_and_get_ptr_ref());
		if (0 > err) ec = make_netlink_error_code(err);
		return res;
	}

	rtnl_link_ref rtnl_link_ref::get_link(nl_socket_ptr const& sock, char const* name, std::error_code& ec) noexcept {
		rtnl_link_ref res;
		int err = rtnl_link_get_kernel(sock.get(), 0, name, res.reset_and_get_ptr_ref());
		if (0 > err) ec = make_netlink_error_code(err);
		return res;
	}

	void rtnl_link_ref::set_qdisc(QString const& qdisc) const {
		::rtnl_link_set_qdisc(get(), qdisc.toUtf8().data());
	}
	QString rtnl_link_ref::get_qdisc() const {
		return QString::fromUtf8(::rtnl_link_get_qdisc(get()));
	}

	void rtnl_link_ref::set_name(QString const& name) const {
		::rtnl_link_set_name(get(), name.toUtf8().data());
	}
	QString rtnl_link_ref::get_name() const {
		return QString::fromUtf8(::rtnl_link_get_name(get()));
	}

	void rtnl_link_ref::set_group(uint32_t group) const noexcept {
		::rtnl_link_set_group(get(), group);
	}
	uint32_t rtnl_link_ref::get_group() const noexcept {
		return ::rtnl_link_get_group(get());
	}

	void rtnl_link_ref::set_flags(link_flags_t flags) const noexcept {
		::rtnl_link_set_flags(get(), static_cast<int>(flags));
	}
	void rtnl_link_ref::unset_flags(link_flags_t flags) const noexcept {
		::rtnl_link_unset_flags(get(), static_cast<int>(flags));
	}
	link_flags_t rtnl_link_ref::get_flags() const noexcept {
		return link_flags_t(::rtnl_link_get_flags(get()));
	}

	void rtnl_link_ref::set_mtu(uint mtu) const noexcept {
		::rtnl_link_set_mtu(get(), mtu);
	}
	uint rtnl_link_ref::get_mtu() const noexcept {
		return ::rtnl_link_get_mtu(get());
	}

	void rtnl_link_ref::set_txqlen(uint txqlen) const noexcept {
		::rtnl_link_set_txqlen(get(), txqlen);
	}
	uint rtnl_link_ref::get_txqlen() const noexcept {
		return ::rtnl_link_get_txqlen(get());
	}

	void rtnl_link_ref::set_ifindex(int ifIndex) const noexcept {
		::rtnl_link_set_ifindex(get(), ifIndex);
	}
	int rtnl_link_ref::get_ifindex() const noexcept {
		return ::rtnl_link_get_ifindex(get());
	}

	void rtnl_link_ref::set_family(int family) const noexcept {
		::rtnl_link_set_family(get(), family);
	}
	int rtnl_link_ref::get_family() const noexcept {
		return ::rtnl_link_get_family(get());
	}

	void rtnl_link_ref::set_arptype(uint arpType) const noexcept {
		::rtnl_link_set_arptype(get(), arpType);
	}
	uint rtnl_link_ref::get_arptype() const noexcept {
		return ::rtnl_link_get_arptype(get());
	}

	void rtnl_link_ref::set_addr(nl_addr_ref addr) const noexcept {
		::rtnl_link_set_addr(get(), addr.get());
	}
	nl_addr_ref rtnl_link_ref::get_addr() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_link_get_addr(get()));
	}

	void rtnl_link_ref::set_broadcast(nl_addr_ref addr) const noexcept {
		::rtnl_link_set_broadcast(get(), addr.get());
	}
	nl_addr_ref rtnl_link_ref::get_broadcast() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_link_get_broadcast(get()));
	}

	void rtnl_link_ref::set_link(int link) const noexcept {
		::rtnl_link_set_link(get(), link);
	}
	int rtnl_link_ref::get_link() const noexcept {
		return ::rtnl_link_get_link(get());
	}

	void rtnl_link_ref::set_master(int master) const noexcept {
		::rtnl_link_set_master(get(), master);
	}
	int rtnl_link_ref::get_master() const noexcept {
		return ::rtnl_link_get_master(get());
	}

	void rtnl_link_ref::set_carrier(carrier_t carrier) const noexcept {
		::rtnl_link_set_carrier(get(), static_cast<uint8_t>(carrier));
	}
	carrier_t rtnl_link_ref::get_carrier() const noexcept {
		return static_cast<carrier_t>(::rtnl_link_get_carrier(get()));
	}

	void rtnl_link_ref::set_operstate(operstate_t operstate) const noexcept {
		::rtnl_link_set_operstate(get(), static_cast<int>(operstate));
	}
	operstate_t rtnl_link_ref::get_operstate() const noexcept {
		return static_cast<operstate_t>(::rtnl_link_get_operstate(get()));
	}

	void rtnl_link_ref::set_linkmode(linkmode_t linkmode) const noexcept {
		::rtnl_link_set_linkmode(get(), static_cast<uint8_t>(linkmode));
	}
	linkmode_t rtnl_link_ref::get_linkmode() const noexcept {
		return static_cast<linkmode_t>(::rtnl_link_get_linkmode(get()));
	}

	void rtnl_link_ref::set_link_netnsid(int32_t link_netnsid) const noexcept {
		::rtnl_link_set_link_netnsid(get(), link_netnsid);
	}
	int32_t rtnl_link_ref::get_link_netnsid(std::error_code& ec) const noexcept {
		int32_t link_netnsid{0};
		int err = ::rtnl_link_get_link_netnsid(get(), &link_netnsid);
		if (0 > err) {
			ec = make_netlink_error_code(err);
		}
		return link_netnsid;
	}

	void rtnl_link_ref::set_ifalias(const QString& ifAlias) const noexcept {
		::rtnl_link_set_ifalias(get(), ifAlias.toUtf8().data());
	}
	QString rtnl_link_ref::get_ifalias() const noexcept {
		return QString::fromUtf8(::rtnl_link_get_ifalias(get()));
	}

	uint32_t rtnl_link_ref::get_num_vf(std::error_code& ec) const noexcept {
		uint32_t num_vf{0};
		int err = ::rtnl_link_get_num_vf(get(), &num_vf);
		if (0 > err) {
			ec = make_netlink_error_code(err);
		}
		return num_vf;
	}

	std::error_code rtnl_link_ref::set_stat(rtnl_link_stat_id_t id, uint64_t value) const noexcept {
		int err = ::rtnl_link_set_stat(get(), id, value);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	uint64_t rtnl_link_ref::get_stat(rtnl_link_stat_id_t id) const noexcept {
		return ::rtnl_link_get_stat(get(), id);
	}

	std::error_code rtnl_link_ref::set_type(QString const& type) const noexcept {
		int err = ::rtnl_link_set_type(get(), type.toUtf8().data());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	QString rtnl_link_ref::get_type() const noexcept {
		return QString::fromUtf8(::rtnl_link_get_type(get()));
	}

	void rtnl_link_ref::set_promiscuity(uint32_t promiscuity) const noexcept {
		::rtnl_link_set_promiscuity(get(), promiscuity);
	}
	uint32_t rtnl_link_ref::get_promiscuity() const noexcept {
		return ::rtnl_link_get_promiscuity(get());
	}

	void rtnl_link_ref::set_num_tx_queues(uint32_t num_tx_queues) const noexcept {
		::rtnl_link_set_num_tx_queues(get(), num_tx_queues);
	}
	uint32_t rtnl_link_ref::get_num_tx_queues() const noexcept {
		return ::rtnl_link_get_num_tx_queues(get());
	}

	void rtnl_link_ref::set_num_rx_queues(uint32_t num_rx_queues) const noexcept {
		::rtnl_link_set_num_rx_queues(get(), num_rx_queues);
	}
	uint32_t rtnl_link_ref::get_num_rx_queues() const noexcept {
		return ::rtnl_link_get_num_rx_queues(get());
	}

	QByteArray rtnl_link_ref::get_phys_port_id() const {
		return from_nl_data(::rtnl_link_get_phys_port_id(get()));
	}

	void rtnl_link_ref::set_ns_fd(int ns_fd) const noexcept {
		::rtnl_link_set_ns_fd(get(), ns_fd);
	}
	int rtnl_link_ref::get_ns_fd() const noexcept {
		return ::rtnl_link_get_ns_fd(get());
	}

	void rtnl_link_ref::set_ns_pid(pid_t ns_pid) const noexcept {
		::rtnl_link_set_ns_pid(get(), ns_pid);
	}
	pid_t rtnl_link_ref::get_ns_pid() const noexcept {
		return ::rtnl_link_get_ns_pid(get());
	}
}
