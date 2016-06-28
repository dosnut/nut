#include "netlink_sock.h"

#include <vector>

namespace netlink {
	class nl_callback_ref::func_storage {
	public:
		explicit func_storage() noexcept {
		}

		explicit func_storage(std::shared_ptr<func_storage> const& parent) noexcept
		: m_parent(parent) {
		}

		static int handler(::nl_msg* msg, void* arg) {
			auto cb = reinterpret_cast<func_t*>(arg);
			return (*cb)(msg);
		}

		void clear(callback_type type) {
			size_t ndx = static_cast<size_t>(int(type));
			if (ndx >= __NL_CB_TYPE_MAX) return;
			m_funcs[ndx] = func_t();
		}

		void* set_arg(callback_type type, func_t const& cb) {
			size_t ndx = static_cast<size_t>(int(type));
			if (ndx >= __NL_CB_TYPE_MAX) std::abort();
			m_funcs[ndx] = cb;
			return reinterpret_cast<void*>(&m_funcs[ndx]);
		}

		static int err_handler(::sockaddr_nl* nla, ::nlmsgerr* nlerr, void* arg) {
			auto self = reinterpret_cast<func_storage*>(arg);
			return self->m_err_func(nla, nlerr);
		}

		void err_clear() {
			m_err_func = err_func_t();
		}

		void* err_arg(err_func_t const& cb) {
			m_err_func = cb;
			return reinterpret_cast<void*>(this);
		}

	private:
		func_t m_funcs[__NL_CB_TYPE_MAX];
		err_func_t m_err_func;
		// just to keep stuff alive
		std::shared_ptr<func_storage> const m_parent;
	};

	nl_callback_ref nl_callback_ref::alloc(callback_kind kind) noexcept {
		return take_own(::nl_cb_alloc(::nl_cb_kind(int(kind))));
	}
	nl_callback_ref nl_callback_ref::take_own(::nl_cb* cb) noexcept {
		nl_callback_ref res = parent::take_own(cb);
		res.m_storage = std::make_shared<func_storage>();
		return res;
	}
	nl_callback_ref nl_callback_ref::take_inc_ref(::nl_cb* cb) noexcept {
		nl_callback_ref res = parent::take_inc_ref(cb);
		res.m_storage = std::make_shared<func_storage>();
		return res;
	}

	nl_callback_ref nl_callback_ref::clone() const noexcept {
		nl_callback_ref res = parent::clone();
		res.m_storage = std::make_shared<func_storage>(m_storage);
		return res;
	}

	std::error_code nl_callback_ref::set(callback_type type, callback_kind kind) const noexcept {
		m_storage->clear(type);
		int err = ::nl_cb_set(get(), ::nl_cb_type(type), ::nl_cb_kind(kind), nullptr, nullptr);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_callback_ref::set(callback_type type, ::nl_recvmsg_msg_cb_t cb, void* arg) const noexcept {
		m_storage->clear(type);
		int err = ::nl_cb_set(get(), ::nl_cb_type(type), NL_CB_CUSTOM, cb, arg);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_callback_ref::set(callback_type type, func_t const& cb) const noexcept {
		int err = ::nl_cb_set(get(), ::nl_cb_type(type), NL_CB_CUSTOM,
			&func_storage::handler, m_storage->set_arg(type, cb));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_callback_ref::err(callback_kind kind) const noexcept {
		m_storage->err_clear();
		int err = ::nl_cb_err(get(), ::nl_cb_kind(kind), nullptr, nullptr);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_callback_ref::err(::nl_recvmsg_err_cb_t cb, void* arg) const noexcept {
		m_storage->err_clear();
		int err = ::nl_cb_err(get(), NL_CB_CUSTOM, cb, arg);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_callback_ref::err(err_func_t const& cb) const noexcept {
		int err = ::nl_cb_err(get(), NL_CB_CUSTOM,
			&func_storage::err_handler, m_storage->err_arg(cb));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	void nl_socket_ptr::deleter_t::operator()(::nl_sock* sock) const noexcept {
		::nl_socket_free(sock);
	}

	nl_socket_ptr::nl_socket_ptr(::nl_sock* sock) noexcept
	: m_sock(sock) {
		m_callback = nl_callback_ref::take_inc_ref(::nl_socket_get_cb(sock));
	}

	nl_socket_ptr nl_socket_ptr::alloc() noexcept {
		return nl_socket_ptr(::nl_socket_alloc());
	}

	void nl_socket_ptr::reset() noexcept {
		m_sock.reset();
	}

	nl_sock* nl_socket_ptr::get() const noexcept {
		return m_sock.get();
	}

	nl_sock* nl_socket_ptr::release() noexcept {
		return m_sock.release();
	}
	nl_socket_ptr::operator bool() const noexcept {
		return bool(m_sock);
	}

	std::error_code nl_socket_ptr::connect(protocol prot) const noexcept {
		int err = nl_connect(m_sock.get(), int(prot));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	void nl_socket_ptr::close() const noexcept {
		nl_close(m_sock.get());
	}

	std::error_code nl_socket_ptr::connect_groups(std::initializer_list<nfnetlink_group> groups) const noexcept {
		if (auto ec = connect(protocol::netfilter)) return ec;
		for (nfnetlink_group group: groups) {
			if (auto ec = add_membership(group)) return ec;
		}
		return std::error_code{};
	}

	std::error_code nl_socket_ptr::connect_groups(std::initializer_list<rtnetlink_group> groups) const noexcept {
		if (auto ec = connect(protocol::route)) return ec;
		for (rtnetlink_group group: groups) {
			if (auto ec = add_membership(group)) return ec;
		}
		return std::error_code{};
	}

	std::error_code nl_socket_ptr::connect_groups(std::initializer_list<xfrm_nlgroups> groups) const noexcept {
		if (auto ec = connect(protocol::xfrm)) return ec;
		for (xfrm_nlgroups group: groups) {
			if (auto ec = add_membership(group)) return ec;
		}
		return std::error_code{};
	}


	void nl_socket_ptr::set_local_port(uint32_t local_port) const noexcept {
		::nl_socket_set_local_port(m_sock.get(), local_port);
	}
	uint32_t nl_socket_ptr::get_local_port() const noexcept {
		return ::nl_socket_get_local_port(m_sock.get());
	}

	std::error_code nl_socket_ptr::add_membership(nfnetlink_group group) const noexcept {
		return add_membership_raw(int(group));
	}
	std::error_code nl_socket_ptr::drop_membership(nfnetlink_group group) const noexcept {
		return drop_membership_raw(int(group));
	}

	std::error_code nl_socket_ptr::add_membership(rtnetlink_group group) const noexcept {
		return add_membership_raw(int(group));
	}
	std::error_code nl_socket_ptr::drop_membership(rtnetlink_group group) const noexcept {
		return drop_membership_raw(int(group));
	}

	std::error_code nl_socket_ptr::add_membership(xfrm_nlgroups group) const noexcept {
		return add_membership_raw(int(group));
	}
	std::error_code nl_socket_ptr::drop_membership(xfrm_nlgroups group) const noexcept {
		return drop_membership_raw(int(group));
	}

	std::error_code nl_socket_ptr::add_membership_raw(int group) const noexcept {
		int err = ::nl_socket_add_membership(m_sock.get(), group);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	std::error_code nl_socket_ptr::drop_membership_raw(int group) const noexcept {
		int err = ::nl_socket_drop_membership(m_sock.get(), group);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	void nl_socket_ptr::set_peer_port(uint32_t peer_port) const noexcept {
		::nl_socket_set_peer_port(m_sock.get(), peer_port);
	}
	uint32_t nl_socket_ptr::get_peer_port() const noexcept {
		return ::nl_socket_get_peer_port(m_sock.get());
	}

	void nl_socket_ptr::set_peer_groups(uint32_t peer_groups) const noexcept {
		::nl_socket_set_peer_groups(m_sock.get(), peer_groups);
	}
	uint32_t nl_socket_ptr::get_peer_groups() const noexcept {
		return ::nl_socket_get_peer_groups(m_sock.get());
	}

	void nl_socket_ptr::set_cb(nl_callback_ref const& callback) noexcept {
		m_callback = callback;
		::nl_socket_set_cb(m_sock.get(), callback.get());
	}
	nl_callback_ref const& nl_socket_ptr::get_cb() const noexcept {
		return m_callback;
	}

	std::error_code nl_socket_ptr::set_buffer_size(int rxbuf, int txbuf) const noexcept {
		int err = ::nl_socket_set_buffer_size(m_sock.get(), rxbuf, txbuf);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_socket_ptr::set_msg_buf_size(size_t msg_buf_size) const noexcept {
		int err = ::nl_socket_set_msg_buf_size(m_sock.get(), msg_buf_size);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	size_t nl_socket_ptr::get_msg_buf_size() const noexcept {
		return ::nl_socket_get_msg_buf_size(m_sock.get());
	}

	std::error_code nl_socket_ptr::set_passcred(int passcred) const noexcept {
		int err = ::nl_socket_set_passcred(m_sock.get(), passcred);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_socket_ptr::recv_pktinfo(int state) const noexcept {
		int err = ::nl_socket_recv_pktinfo(m_sock.get(), state);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	void nl_socket_ptr::disable_seq_check() const noexcept {
		::nl_socket_disable_seq_check(m_sock.get());
	}
	unsigned int nl_socket_ptr::use_seq() const noexcept {
		return ::nl_socket_use_seq(m_sock.get());
	}
	void nl_socket_ptr::disable_auto_ack() const noexcept {
		m_auto_ack = false;
		::nl_socket_disable_auto_ack(m_sock.get());
	}
	void nl_socket_ptr::enable_auto_ack() const noexcept {
		m_auto_ack = true;
		::nl_socket_enable_auto_ack(m_sock.get());
	}

	std::error_code nl_socket_ptr::set_fd(int protocol, int fd) const noexcept {
		int err = ::nl_socket_set_fd(m_sock.get(), protocol, fd);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	int nl_socket_ptr::get_fd() const noexcept {
		return nl_socket_get_fd(m_sock.get());
	}

	std::error_code nl_socket_ptr::set_nonblocking() const noexcept {
		int err = nl_socket_set_nonblocking(m_sock.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	void nl_socket_ptr::enable_msg_peek() const noexcept {
		::nl_socket_enable_msg_peek(m_sock.get());
	}
	void nl_socket_ptr::disable_msg_peek() const noexcept {
		::nl_socket_disable_msg_peek(m_sock.get());
	}

	std::error_code nl_socket_ptr::recvmsgs(nl_callback_ref const& cb) const noexcept {
		int err = ::nl_recvmsgs(get(), cb.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_socket_ptr::recvmsgs() const noexcept {
		int err = ::nl_recvmsgs_default(get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_socket_ptr::send_simple(int type, int flags, void* buf, size_t size) const noexcept {
		int err = ::nl_send_simple(get(), type, flags, buf, size);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_socket_ptr::send_auto(nl_msg* msg) const noexcept {
		int err = ::nl_send_auto(get(), msg);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code nl_socket_ptr::wait_for_ack() const noexcept {
		if (m_auto_ack) {
			if (int err = ::nl_wait_for_ack(get())) return make_netlink_error_code(err);
		}
		return std::error_code{};
	}
}
