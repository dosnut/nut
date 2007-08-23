#include "common.h"

namespace qnut {
    QString getDeviceIcon(CDevice * device) {
        bool undefined = (device->activeEnvironment->interfaces.isEmpty());
        
        switch (device->type) {
            case ethernet:
                return (QString)(device->enabled ? UI_ICON_ETH_ENABLED : (undefined ? UI_ICON_ETH_UNDEFINED : UI_ICON_ETH_DISABLED));
            case wlan:
                return (QString)(device->enabled ? UI_ICON_AIR_ENABLED : (undefined ? UI_ICON_AIR_UNDEFINED : UI_ICON_AIR_DISABLED));
            case ppp:
                return (QString)(device->enabled ? UI_ICON_PPP_ENABLED : (undefined ? UI_ICON_PPP_UNDEFINED : UI_ICON_PPP_DISABLED));
            default:
                return QString();
        }
    }
};
