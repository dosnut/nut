
EGIT_REPO_URI="git://repo.or.cz/nut.git"

inherit eutils flag-o-matic git

DESCRIPTION="Network Utility like knetworkmanager"
HOMEPAGE="http://repo.or.cz/nut.git"
SRC_URI=""

LICENSE="GPL-2.0"
SLOT="0"
KEYWORDS="~x86"
IUSE="debug"

RDEPEND=">=x11-libs/qt-4.3.2
		sys-apps/dbus"
DEPEND="${RDEPEND}
	>=sys-kernel/linux-headers-2.6
	>=dev-libs/libnl-1.0_pre6
	sys-devel/bison
	sys-devel/flex"

S=${WORKDIR}/${EGIT_PROJECT}

pkg_setup() {
	if ! built_with_use x11-libs/qt dbus ; then
		eerror ">=qt4.3.2 requires dbus"
		die "rebuild >=x11-libs/qt-4.3.2 with the dbus USE flag"
	fi
	if ! qmake --version | grep -i qt4 ; then
		eerror "qmake does not point to qmake-qt4"
		die "Install qmake-qt4 and set symlinks correctly"
	fi
	enewgroup netdev
}


src_unpack() {
	git_src_unpack
}

src_compile() {
	cd "${S}"
	if use debug; then 
		qmake
	else
		qmake -recursive -Wall 'CONFIG+=release' 'DEFINES+=QT_NO_DEBUG_OUTPUT'
	fi
	make
}

src_install() {
	
	make INSTALL_ROOT="${D}" install || die

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

	insinto /etc/dbus-1/system.d/
	newins "${S}"/debian/nuts-dbus.conf nuts-dbus.conf
}

pkg_postinst() {
	elog "Nuts' config file is located at /etc/nuts/nuts.config."
	elog "Edit this file to your needs."
	elog "An example config file can be found at /etc/nuts.config.example."
	elog "If you want to use the wpa_supplicant backend as unprivileged user,"
	elog "you have to set control rights in your wpa_supplicant.conf accordingly:"
	elog "i.e. ctrl_interface_group=netdev"
	elog "To start nuts add nuts to your default runlevel:"
	elog "rc-update add nuts default"
}

