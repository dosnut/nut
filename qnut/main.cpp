#include <QtGui>
#include <QTranslator>
#include "connectionmanager.h"
#define QNUT_TESTING

using namespace qnut;

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;

    translator.load(QString("qnut_") + locale);
    app.installTranslator(&translator);
    app.setQuitOnLastWindowClosed(false);

    CConnectionManager mainwin;
#ifdef QNUT_TESTING
    CDevice * testdev0 = new CDevice(NULL);
    testdev0->properties.name = "eth0";
    testdev0->properties.enabled = false;
    testdev0->properties.type = ethernet;
    mainwin.deviceManager.devices.append(testdev0);
    mainwin.uiAddedDevice(testdev0);
    
    CDevice * testdev1 = new CDevice(NULL);
    testdev1->properties.name = "wlan0";
    testdev1->properties.enabled = true;
    testdev1->properties.type = wlan;
    mainwin.deviceManager.devices.append(testdev1);
    mainwin.uiAddedDevice(testdev1);
    
    int exitcode = app.exec();
    
    mainwin.deviceManager.devices.removeAll(testdev0);
    mainwin.uiRemovedDevice(testdev0);
    delete testdev0;
    
    mainwin.deviceManager.devices.removeAll(testdev1);
    mainwin.uiRemovedDevice(testdev1);
    delete testdev1;
    
    return exitcode;
#else
    return app.exec();
#endif
}
