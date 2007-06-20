#include "connman.h"

namespace qnut {
    //CConnectionManager
    CConnectionManager::CConnectionManager(QWidget * parent) : QMainWindow(parent), trayicon(this) {
        ui.setupUi(this);
        trayicon.show();
    }
    
    void CConnectionManager::uiDevicesUpdate(QString name) {
        ui.
    }
};
