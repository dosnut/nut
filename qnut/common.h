#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <libnut/libnut_cli.h>
#include <QString>
#include <QAction>
#include "constants.h"

namespace qnut {
    using namespace libnut;
    QString getDeviceIcon(CDevice * device);
    QAction * getSeparator(QObject * parent);
    QString activeIP(CDevice * device);
};
#endif
