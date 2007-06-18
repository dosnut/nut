#include "connman.h"

namespace qnut {
    CConnectionManager::CConnectionManager(QWidget * parent) : QMainWindow(parent), trayicon(this) {
        ui.setupUi(this);
    }
};
