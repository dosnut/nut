#ifndef QNUT_CONSTANTS_H
#define QNUT_CONSTANTS_H

#define UI_VERSION                      "0.1.1"
#define UI_NAME                         tr("QNUT - Qt client for Network UTility Server (NUTS)")

#ifdef QNUT_RELEASE
#define UI_DIR_ICONS                    "/usr/share/qnut/icons/"
#define UI_DIR_TRANSLATIONS             "/usr/share/qnut/lang/"
#else
#define UI_DIR_ICONS                    "res/"
#define UI_DIR_TRANSLATIONS             ""
#endif
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

#define UI_ICON_SYSTRAY                 UI_DIR_ICONS "network.png"

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

#define UI_ICON_DEVICE_ENABLE           UI_DIR_ICONS "device_enable.png"
#define UI_ICON_DEVICE_DISABLE          UI_DIR_ICONS "device_disable.png"
#define UI_ICON_DEVICE_SETTINGS         UI_DIR_ICONS "device_settings.png"

#define UI_ICON_ENVIRONMENT             UI_DIR_ICONS "environment.png"
#define UI_ICON_ENVIRONMENT_ENTER       UI_DIR_ICONS "environment_enter.png"

#define UI_ICON_INTERFACE               UI_DIR_ICONS "interface.png"
#define UI_ICON_INTERFACE_ACTIVATE      UI_DIR_ICONS "interface_activate.png"
#define UI_ICON_INTERFACE_DEACTIVATE    UI_DIR_ICONS "interface_deactivate.png"

#define UI_ICON_EDIT                    UI_DIR_ICONS "edit.png"
#define UI_ICON_REFRESH                 UI_DIR_ICONS "refresh.png"

#define UI_FLAG_SCRIPT_NONE             0x00
#define UI_FLAG_SCRIPT_UP               0x01
#define UI_FLAG_SCRIPT_UNCONFIGURED     0x02
#define UI_FLAG_SCRIPT_CARRIER          0x04
#define UI_FLAG_SCRIPT_ACTIVATED        0x08
#define UI_FLAG_SCRIPT_DEACTIVATED      0x10

#endif
