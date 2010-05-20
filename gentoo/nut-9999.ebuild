# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EGIT_REPO_URI="git://stbuehler.de/nut.git"

inherit git bash-completion cmake-utils

DESCRIPTION="An advanced network manager with event based script execution"
HOMEPAGE="http://redmine.stbuehler.de/projects/show/nut"
SRC_URI=""

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE="debug wifi X"

RDEPEND="
	X? ( >=x11-libs/qt-gui-4.4.0 )
	X? ( >=x11-libs/qt-svg-4.4.0 )
	>=x11-libs/qt-dbus-4.4.0
	>=x11-libs/qt-core-4.4.0
	sys-apps/dbus
	wifi? ( >=net-wireless/wpa_supplicant-0.6.0 )
	wifi? ( >=net-wireless/wireless-tools-29 )
	bash-completion? ( app-shells/bash-completion )"
DEPEND="${RDEPEND}
	sys-kernel/linux-headers
	>=dev-libs/libnl-1.1-r1
	sys-devel/bison
	sys-devel/flex
	>=dev-util/cmake-2.8.1"

S=${WORKDIR}/${EGIT_PROJECT}

pkg_setup() {
	enewgroup netdev
}

src_unpack() {
	# check version:
	if [ "$PV" != 9999 ]; then
		EGIT_TREE="v${PV}"
	fi
	git_src_unpack
}

src_configure() {
	if ! use debug; then
		append-cppflags -DQT_NO_DEBUG_OUTPUT
	fi

	if ! use wifi; then
		append-cppflags -DLIBNUT_NO_WIRELESS
		append-cppflags -DQNUT_NO_WIRELESS
	fi

	cmake-utils_src_configure
}

src_install() {
	cmake-utils_src_install
	exeinto /etc/init.d/
	newexe "${S}"/gentoo/nuts.init nuts
	dodir /etc/nuts
	insinto /etc/nuts/
	newins "${S}"/docs/config.example nuts.config.example

	exeinto /etc/nuts/
	newexe "${S}"/nuts/dispatch dispatch
	dodir /etc/nuts/events
	dodir /etc/nuts/events/all
	dodir /etc/nuts/events/default
	insinto /etc/nuts/events/all
	newins "${S}"/gentoo/start_avahi start_avahi
	newins "${S}"/gentoo/ntp-date ntp-date
	newins "${S}"/gentoo/autoswitch_netcards autoswitch_netcards

	insinto /etc/dbus-1/system.d/
	newins "${S}"/resources/nuts-dbus.conf nuts-dbus.conf

	dobashcompletion "${S}"/resources/cnut.bash_completion cnut

	doman "${S}"/resources/*.1
}

pkg_postinst() {
	bash-completion_pkg_postinst

	elog "Nuts' config file is located at /etc/nuts/nuts.config."
	elog "Edit this file to your needs."
	elog "An example config file can be found at /etc/nuts.config.example."
	elog ""
	elog "You have to be in the group netdev in order to use qnut"
	elog "If you want to use the wpa_supplicant backend as unprivileged user,"
	elog "you have to set control rights in your wpa_supplicant.conf accordingly:"
	elog "i.e. ctrl_interface_group=netdev"
	elog ""
	elog "To start nuts add nuts to your default runlevel:"
	elog "rc-update add nuts default"
	elog ""
	elog "nuts as well as qnut support event based script execution"
	elog "Have look at /etc/nuts/dispatch to see how to write your own (server) scripts and where to put them."
	elog "For qnut, please have a look at the man page"
}
