
EGIT_REPO_URI="git://repo.or.cz/nut.git"

inherit eutils flag-o-matic git

DESCRIPTION="An advanced network manager with event based script execution"
HOMEPAGE="http://repo.or.cz/nut.git"
SRC_URI=""

LICENSE="GPL-2.0"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE="debug wifi X"

RDEPEND=">=x11-libs/qt-4.3.2
		sys-apps/dbus
		wifi? ( >=net-wireless/wpa_supplicant-0.6.0 )"
DEPEND="${RDEPEND}
	>=sys-kernel/linux-headers-2.6.23-r2
	>=dev-libs/libnl-1.0_pre6-r1
	sys-devel/bison
	sys-devel/flex"

S=${WORKDIR}/${EGIT_PROJECT}

pkg_setup() {
	if ! built_with_use x11-libs/qt dbus ; then
		eerror "Nut requires >=qt4.3.2 to be built with dbus"
		die "rebuild x11-libs/qt with the dbus USE flag"
	fi
	if ! qmake --version | grep -i qt4 ; then
		eerror "qmake does not point to qmake-qt4"
		die "Install qmake-qt4 and set symlinks correctly"
	fi
	enewgroup netdev
}


src_unpack() {
	git_src_unpack

	epatch "$S"/gentoo/files/gentoo_linux_headers.diff || die 
}

src_compile() {
	cd "${S}"
	config_defines = ""
	config_release = ""
	if use debug; then 
		config_release="debug"
	else
		config_release="release"
		config_defines="$config_defines QT_NO_DEBUG_OUTPUT"
	fi

	if ! use wifi; then
		config_defines="$config_defines LIBNUT_NO_WIRELESS QNUT_NO_WIRELESS"
	fi
	
	if ! use X; then
		#Patch nut.pro: we don't need libnutclient,libnutwireless and qnut
		sed --in-place -e :a -e "s/libnutwireless\|libnutclient\|qnut//;ta" nut.pro
	fi
	
	qmake -recursive -Wall "CONFIG+=$config_release" "DEFINES+=$config_defines"
	make || die
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
	newins "${S}"/gentoo/ntp-date ntp-date
	newins "${S}"/gentoo/autoswitch_netcards autoswitch_netcards

	insinto /etc/dbus-1/system.d/
	newins "${S}"/debian/nuts-dbus.conf nuts-dbus.conf
}

pkg_postinst() {
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
	elog ""
	elog "Known bugs:"
	elog "Qnut/cnut will use 99% cpu if you are not allowed to access nuts' dbus interface."
	elog "This will be fixed in qt-4.4"
	elog "Configuring networks is not fully tested. Please make a back-up before you activate wpa_supplicant's config write feature."
}
