#ifndef QNUT_CONSTANTS_H
#define QNUT_CONSTANTS_H

#define UI_VERSION                      "0.5.2"
#define UI_NAME                         tr("QNUT - Qt client for Network UTility Server (NUTS)")

#define UI_STRING_ORGANIZATION          "nut"
#define UI_STRING_APPNAME               "qnut"

#define UI_DIR_WORK                     ".qnut/"

#define UI_DIR_SCRIPT_UP                "up/"
#define UI_DIR_SCRIPT_UNCONFIGURED      "unconfigured/"
#define UI_DIR_SCRIPT_CARRIER           "carrier/"
#define UI_DIR_SCRIPT_ACTIVATED         "activated/"
#define UI_DIR_SCRIPT_DEACTIVATED       "deactivated/"

#define UI_PATH_TRANSLATIONS            "/usr/share/qnut/lang/"
#define UI_PATH_ICONS                   "/usr/share/qnut/icons/"
#define UI_PATH_WORK                    (QDir::toNativeSeparators(QDir::homePath()) + "/" UI_DIR_WORK)
#define UI_PATH_DEV(a)                  ((UI_PATH_WORK + a) + '/')

#define UI_FILE_LOG                     (UI_PATH_WORK + "qnut.log")
#define UI_FILE_CONFIG                  (UI_PATH_WORK + "qnut.conf")

#define UI_ICON_QNUT                    UI_PATH_ICONS "qnut.svg"
#define UI_ICON_QNUT_SMALL              UI_PATH_ICONS "qnut_small.svg"

#define UI_ICON_ENABLE                  UI_PATH_ICONS "enable.png"
#define UI_ICON_ENABLE_ALL              UI_PATH_ICONS "enable_all.png"
#define UI_ICON_DISABLE                 UI_PATH_ICONS "disable.png"

#define UI_ICON_SELECTED                UI_PATH_ICONS "selected.png"
#define UI_ICON_UNSELECTED              UI_PATH_ICONS "unselected.png"

#define UI_ICON_SCRIPT_SETTINGS         UI_PATH_ICONS "script_settings.png"
#define UI_ICON_CONFIGURE               UI_PATH_ICONS "configure.png"
#define UI_ICON_EDIT                    UI_PATH_ICONS "edit.png"
#define UI_ICON_RELOAD                  UI_PATH_ICONS "reload.png"
#define UI_ICON_CLEAR                   UI_PATH_ICONS "clear.png"
#define UI_ICON_SAVE                    UI_PATH_ICONS "save.png"

#define UI_ICON_ADD                     UI_PATH_ICONS "add.png"
#define UI_ICON_REMOVE                  UI_PATH_ICONS "remove.png"

#define UI_ICON_ADD_ADHOC               UI_PATH_ICONS "add_adhoc.png"

#define UI_ICON_FORCE                   UI_PATH_ICONS "force.png"
#define UI_ICON_WARNING                 UI_PATH_ICONS "warning.png"
#define UI_ICON_SEARCH                  UI_PATH_ICONS "search.png"

#define UI_ICON_AIR                     UI_PATH_ICONS "air.png"
#define UI_ICON_ADHOC                   UI_PATH_ICONS "adhoc.png"

#define UI_ICON_AIR_DOWN                UI_PATH_ICONS "air_down.png"
#define UI_ICON_ADHOC_DOWN              UI_PATH_ICONS "adhoc_down.png"
#define UI_ICON_ADHOC_ACTIVATED         UI_PATH_ICONS "adhoc_activated.png"

#define UI_ICON_DETAILED                UI_PATH_ICONS "view_detailed.png"

#define UI_ICON_ETH_UP                  UI_PATH_ICONS "eth_up.png"
#define UI_ICON_ETH_UNCONFIGURED        UI_PATH_ICONS "eth_unconfigured.png"
#define UI_ICON_ETH_CARRIER             UI_PATH_ICONS "eth_carrier.png"
#define UI_ICON_ETH_ACTIVATED           UI_PATH_ICONS "eth_activated.png"
#define UI_ICON_ETH_DEACTIVATED         UI_PATH_ICONS "eth_deactivated.png"

#define UI_ICON_AIR_UP                  UI_PATH_ICONS "air_up.png"
#define UI_ICON_AIR_UNCONFIGURED        UI_PATH_ICONS "air_unconfigured.png"
#define UI_ICON_AIR_CARRIER             UI_PATH_ICONS "air_carrier.png"
#define UI_ICON_AIR_ACTIVATED           UI_PATH_ICONS "air_activated.png"
#define UI_ICON_AIR_DEACTIVATED         UI_PATH_ICONS "air_deactivated.png"

#define UI_ICON_BRIDGE_UP               UI_PATH_ICONS "bridge_up.png"
#define UI_ICON_BRIDGE_UNCONFIGURED     UI_PATH_ICONS "bridge_unconfigured.png"
#define UI_ICON_BRIDGE_CARRIER          UI_PATH_ICONS "bridge_carrier.png"
#define UI_ICON_BRIDGE_ACTIVATED        UI_PATH_ICONS "bridge_activated.png"
#define UI_ICON_BRIDGE_DEACTIVATED      UI_PATH_ICONS "bridge_deactivated.png"

#define UI_ICON_ENVIRONMENT             UI_PATH_ICONS "environment.png"

#define UI_ICON_INTERFACE               UI_PATH_ICONS "interface.png"
#define UI_ICON_INTERFACE_ACTIVE        UI_PATH_ICONS "interface_active.png"

#define UI_FLAG_SCRIPT_NONE             0x00
#define UI_FLAG_SCRIPT_UP               0x01
#define UI_FLAG_SCRIPT_UNCONFIGURED     0x02
#define UI_FLAG_SCRIPT_CARRIER          0x04
#define UI_FLAG_SCRIPT_ACTIVATED        0x08
#define UI_FLAG_SCRIPT_DEACTIVATED      0x10

#define UI_ACTIONS_OVERVIEW             0x00
#define UI_ACTIONS_LOG                  0x01
#define UI_ACTIONS_DEVICE               0x02

#define UI_SETTINGS_MAIN                "Main"
#define UI_SETTINGS_SHOWBALLOONTIPS     "showBalloonTips"
#define UI_SETTINGS_SHOWLOG             "showLog"

#define UI_SETTINGS_CONNECTIONMANAGER   "ConnectionManager"
#define UI_SETTINGS_SIZE                "size"
#define UI_SETTINGS_POS                 "pos"
#define UI_SETTINGS_TOOLBARAREA         "toolBarArea"
#define UI_SETTINGS_SHOWTOOLBAR         "showToolBar"

#define UI_SETIINGS_SCRIPTFLAGS         "scriptFlags"
#define UI_SETTINGS_SHOWTRAYICON        "showTrayIcon"
#define UI_SETTINGS_SHOWDETAILS         "showDetails"

#define UI_SETTINGS_WIRELESSSETTINGS    "WirelessSettings"

#define UI_SETTINGS_IPCONFIGURATIONS    "IPconfigurations"
#define UI_SETTINGS_IP                  "ip"
#define UI_SETTINGS_NETMASK             "netmask"
#define UI_SETTINGS_GATEWAY             "gateway"
#define UI_SETTINGS_DNSSERVERS          "dnsServers"
#define UI_SETTINGS_ADDRESS             "address"

#endif
