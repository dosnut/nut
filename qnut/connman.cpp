#include "connman.h"

namespace qnut {
    //CConnectionManager
    CConnectionManager::CConnectionManager(QWidget * parent) : QMainWindow(parent), trayicon(this) {
        ui.setupUi(this);
        trayicon.show();
    }
    
    void CConnectionManager::uiUpdateDevices(QStringList * names) {
        QStringList::const_iterator i;
        QListWidgetItem * newItem;
        for (i = names->begin(); i != names->end(); i++) {
            newItem = new QListWidgetItem(*i, ui.devicesWidget);
        }
    }
};
