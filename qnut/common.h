#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include "constants.h"
#include "libnut_cli.h"
#include <QString>

namespace qnut {
    using namespace libnut;
    QString getDeviceIcon(CDevice * device);
};
#endif
