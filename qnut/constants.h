#ifndef QNUT_CONSTANTS_H
#define QNUT_CONSTANTS_H

#define UI_VERSION                      "0.4.9"
#define UI_NAME                         tr("QNUT - Qt client for Network UTility Server (NUTS)")

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

#define UI_ICON_ENABLE                  UI_PATH_ICONS "enable.svg"
#define UI_ICON_ENABLE_ALL              UI_PATH_ICONS "enable_all.svg"
#define UI_ICON_DISABLE                 UI_PATH_ICONS "disable.svg"

#define UI_ICON_SELECTED                UI_PATH_ICONS "selected.svg"
#define UI_ICON_UNSELECTED              UI_PATH_ICONS "unselected.svg"

#define UI_ICON_SCRIPT_SETTINGS         UI_PATH_ICONS "script_settings.svg"
#define UI_ICON_CONFIGURE               UI_PATH_ICONS "configure.svg"
#define UI_ICON_EDIT                    UI_PATH_ICONS "edit.svg"
#define UI_ICON_RELOAD                  UI_PATH_ICONS "reload.svg"
#define UI_ICON_CLEAR                   UI_PATH_ICONS "clear.svg"
#define UI_ICON_SAVE                    UI_PATH_ICONS "save.svg"

#define UI_ICON_ADD                     UI_PATH_ICONS "add.svg"
#define UI_ICON_REMOVE                  UI_PATH_ICONS "remove.svg"

#define UI_ICON_ADD_ADHOC               UI_PATH_ICONS "add_adhoc.svg"

#define UI_ICON_FORCE                   UI_PATH_ICONS "force.svg"
#define UI_ICON_WARNING                 UI_PATH_ICONS "warning.svg"
#define UI_ICON_SEARCH                  UI_PATH_ICONS "search.svg"

#define UI_ICON_AIR                     UI_PATH_ICONS "air.svg"
#define UI_ICON_ADHOC                   UI_PATH_ICONS "adhoc.svg"

#define UI_ICON_DETAILED                UI_PATH_ICONS "view_detailed"

#define UI_ICON_ETH_UP                  UI_PATH_ICONS "eth_up.svg"
#define UI_ICON_ETH_UNCONFIGURED        UI_PATH_ICONS "eth_unconfigured.svg"
#define UI_ICON_ETH_CARRIER             UI_PATH_ICONS "eth_carrier.svg"
#define UI_ICON_ETH_ACTIVATED           UI_PATH_ICONS "eth.svg"
#define UI_ICON_ETH_DEACTIVATED         UI_PATH_ICONS "eth_deactivated.svg"

#define UI_ICON_AIR_UP                  UI_PATH_ICONS "air_up.svg"
#define UI_ICON_AIR_UNCONFIGURED        UI_PATH_ICONS "air_unconfigured.svg"
#define UI_ICON_AIR_CARRIER             UI_PATH_ICONS "air_carrier.svg"
#define UI_ICON_AIR_ACTIVATED           UI_PATH_ICONS "air.svg"
#define UI_ICON_AIR_DEACTIVATED         UI_PATH_ICONS "air_deactivated.svg"

#define UI_ICON_ENVIRONMENT             UI_PATH_ICONS "environment.svg"

#define UI_ICON_INTERFACE               UI_PATH_ICONS "interface.svg"
#define UI_ICON_INTERFACE_ACTIVE        UI_PATH_ICONS "interface_active.svg"

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
