#ifndef QNUT_CONSTANTS_H
#define QNUT_CONSTANTS_H

#define UI_VERSION                      "0.3.0-rc2"
#define UI_NAME                         tr("QNUT - Qt client for Network UTility Server (NUTS)")

//#ifdef QNUT_RELEASE
#define UI_DIR_ICONS                    "/usr/share/qnut/icons/"
#define UI_DIR_TRANSLATIONS             "/usr/share/qnut/lang/"
//#else
//#define UI_DIR_ICONS                    "res/"
//#define UI_DIR_TRANSLATIONS             ""
//#endif
#define UI_DIR_WORK                     ".qnut/"
#define UI_PATH_WORK                    (QDir::toNativeSeparators(QDir::homePath()) + "/" UI_DIR_WORK)

#define UI_DIR_SCRIPT_UP                "up/"
#define UI_DIR_SCRIPT_UNCONFIGURED      "unconfigured/"
#define UI_DIR_SCRIPT_CARRIER           "carrier/"
#define UI_DIR_SCRIPT_ACTIVATED         "activated/"
#define UI_DIR_SCRIPT_DEACTIVATED       "deactivated/"
#define UI_PATH_DEV(a)                  ((UI_PATH_WORK + a) + '/')

#define UI_FILE_LOG                     (UI_PATH_WORK + "qnut.log")
#define UI_FILE_CONFIG                  (UI_PATH_WORK + "qnut.conf")

#define UI_ICON_QNUT                    UI_DIR_ICONS "qnut.png"

#define UI_ICON_ENABLE                  UI_DIR_ICONS "enable.png"
#define UI_ICON_ENABLE_ALL              UI_DIR_ICONS "enable_all.png"
#define UI_ICON_DISABLE                 UI_DIR_ICONS "disable.png"

#define UI_ICON_SELECTED                UI_DIR_ICONS "selected.png"
#define UI_ICON_UNSELECTED              UI_DIR_ICONS "unselected.png"

#define UI_ICON_SCRIPT_SETTINGS         UI_DIR_ICONS "script_settings.png"
#define UI_ICON_CONFIGURE               UI_DIR_ICONS "configure.png"
#define UI_ICON_EDIT                    UI_DIR_ICONS "edit.png"
#define UI_ICON_RELOAD                  UI_DIR_ICONS "reload.png"
#define UI_ICON_CLEAR                   UI_DIR_ICONS "clear.png"
#define UI_ICON_SAVE                    UI_DIR_ICONS "save.png"

#define UI_ICON_ADD                     UI_DIR_ICONS "add.png"
#define UI_ICON_REMOVE                  UI_DIR_ICONS "remove.png"

#define UI_ICON_ADD_ADHOC               UI_DIR_ICONS "add_adhoc.png"

#define UI_ICON_FORCE                   UI_DIR_ICONS "force.png"
#define UI_ICON_WARNING                 UI_DIR_ICONS "warning.png"
#define UI_ICON_SEARCH                  UI_DIR_ICONS "search.png"

#define UI_ICON_AIR                     UI_DIR_ICONS "air.png"
#define UI_ICON_ADHOC                   UI_DIR_ICONS "adhoc.png"

#define UI_ICON_DETAILED                UI_DIR_ICONS "view_detailed"

#define UI_ICON_ETH_UP                  UI_DIR_ICONS "eth_up.png"
#define UI_ICON_ETH_UNCONFIGURED        UI_DIR_ICONS "eth_unconfigured.png"
#define UI_ICON_ETH_CARRIER             UI_DIR_ICONS "eth_carrier.png"
#define UI_ICON_ETH_ACTIVATED           UI_DIR_ICONS "eth_activated.png"
#define UI_ICON_ETH_DEACTIVATED         UI_DIR_ICONS "eth_deactivated.png"

#define UI_ICON_AIR_UP                  UI_DIR_ICONS "air_up.png"
#define UI_ICON_AIR_UNCONFIGURED        UI_DIR_ICONS "air_unconfigured.png"
#define UI_ICON_AIR_CARRIER             UI_DIR_ICONS "air_carrier.png"
#define UI_ICON_AIR_ACTIVATED           UI_DIR_ICONS "air_activated.png"
#define UI_ICON_AIR_DEACTIVATED         UI_DIR_ICONS "air_deactivated.png"

#define UI_ICON_ENVIRONMENT             UI_DIR_ICONS "environment.png"

#define UI_ICON_INTERFACE               UI_DIR_ICONS "interface.png"
#define UI_ICON_INTERFACE_ACTIVE        UI_DIR_ICONS "interface_active.png"

#define UI_FLAG_SCRIPT_NONE             0x00
#define UI_FLAG_SCRIPT_UP               0x01
#define UI_FLAG_SCRIPT_UNCONFIGURED     0x02
#define UI_FLAG_SCRIPT_CARRIER          0x04
#define UI_FLAG_SCRIPT_ACTIVATED        0x08
#define UI_FLAG_SCRIPT_DEACTIVATED      0x10

#define UI_ACTIONS_OVERVIEW             0x00
#define UI_ACTIONS_LOG                  0x01
#define UI_ACTIONS_DEVICE               0x02

#endif
