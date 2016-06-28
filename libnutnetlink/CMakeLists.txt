# nutnetlink: libnl c++ wrapper
add_library(nutnetlink SHARED
	netlink.cpp
	netlink.h
	netlink_addr.cpp
	netlink_addr.h
	netlink_cache.cpp
	netlink_cache.h
	netlink_cache_impl.h
	netlink_generic.cpp
	netlink_generic.h
	netlink_msg.cpp
	netlink_msg.h
	netlink_others.cpp
	netlink_others.h
	netlink_rtnl_addr.cpp
	netlink_rtnl_addr.h
	netlink_rtnl_link.cpp
	netlink_rtnl_link.h
	netlink_rtnl_route.cpp
	netlink_rtnl_route.h
	netlink_sock.cpp
	netlink_sock.h
	netlink_templates.h
	netlink_templates_impl.h
)

target_link_libraries(nutnetlink
	nutcommon
	libnl
	libnl-genl
	libnl-route
)

file(GLOB NUTNETLINK_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

install(FILES ${NUTNETLINK_H} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnutnetlink)
