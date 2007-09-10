#include "common.h"

namespace qnut {
    QString getDeviceIcon(CDevice * device) {
        switch (device->type) {
        case DT_ETH:
            switch (device->state) {
            case DS_UP:             return QString(UI_ICON_ETH_UP);
            case DS_UNCONFIGURED:   return QString(UI_ICON_ETH_UNCONFIGURED);
            case DS_CARRIER:        return QString(UI_ICON_ETH_CARRIER);
            case DS_ACTIVATED:      return QString(UI_ICON_ETH_ACTIVATED);
            case DS_DEACTIVATED:    return QString(UI_ICON_ETH_DEACTIVATED);
            default:                break;
            }
        case DT_AIR:
            switch (device->state) {
            case DS_UP:             return QString(UI_ICON_AIR_UP);
            case DS_UNCONFIGURED:   return QString(UI_ICON_AIR_UNCONFIGURED);
            case DS_CARRIER:        return QString(UI_ICON_AIR_CARRIER);
            case DS_ACTIVATED:      return QString(UI_ICON_AIR_ACTIVATED);
            case DS_DEACTIVATED:    return QString(UI_ICON_AIR_DEACTIVATED);
            default:                break;
            }
        default:
            break;
        }
        return QString();
    }

    QAction * getSeparator(QObject * parent) {
        QAction * separator = new QAction(parent);
        separator->setSeparator(true);
        return separator;
    }
};
