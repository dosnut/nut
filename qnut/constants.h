#ifndef QNUT_CONSTANTS_H
#define QNUT_CONSTANTS_H

//Clientversionsschlüssel: main.minor.features version
#define UI_VERSION                      "0.1.0"
#define UI_NAME                         tr("Qt client for Network UTility (NUT)")

#ifdef QNUT_RELEASE
#define UI_DIR_ICONS                    "/usr/share/qnut/icons/"
#define UI_DIR_TRANSLATIONS             "/usr/share/qnut/lang/"
#else
#define UI_DIR_ICONS                    "res/"
#define UI_DIR_TRANSLATIONS             ""
#endif
#define UI_DIR_HOME                     "~/"
#define UI_DIR_WORK                     ".qnut/"
#define UI_DIR_WORK_ABS                 UI_DIR_HOME UI_DIR_WORK

#define UI_FILE_LOG                     UI_DIR_WORK_ABS "qnut.log"
#define UI_FILE_CONFIG                  UI_DIR_WORK_ABS "qnut.conf"

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
#define UI_ICON_AIR_DEACTIVATED         UI_DIR_ICONS "air_deactived.png"

#define UI_ICON_DEVICE_ENABLE           UI_DIR_ICONS "device_enable.png"
#define UI_ICON_DEVICE_DISABLE          UI_DIR_ICONS "device_disable.png"

#define UI_ICON_ENVIRONMENT             UI_DIR_ICONS "environment.png"
#define UI_ICON_ENVIRONMENT_ENTER       UI_DIR_ICONS "environment_enter.png"

#define UI_ICON_INTERFACE               UI_DIR_ICONS "interface.png"
#define UI_ICON_INTERFACE_ACTIVATE      UI_DIR_ICONS "interface_activate.png"
#define UI_ICON_INTERFACE_DEACTIVATE    UI_DIR_ICONS "interface_deactivate.png"

#define UI_ICON_EDIT                    UI_DIR_ICONS "edit.png"
#define UI_ICON_REFRESH                 UI_DIR_ICONS "refresh.png"

#endif
