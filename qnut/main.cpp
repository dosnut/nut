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
    testdev0->properties.activeEnvironment = 0;
    mainwin.deviceManager.devices.append(testdev0);
    CEnvironment * testenv1 = new CEnvironment(testdev0);
    testenv1->properties.name = "default";
    testenv1->properties.active = false;
    testenv1->properties.currentSelection.type = user;
    testdev0->environments.append(testenv1);
    mainwin.uiAddedDevice(testdev0);
    
    CDevice * testdev1 = new CDevice(NULL);
    testdev1->properties.name = "wlan0";
    testdev1->properties.enabled = true;
    testdev1->properties.type = wlan;
    testdev1->properties.activeEnvironment = 0;
    mainwin.deviceManager.devices.append(testdev1);
    CEnvironment * testenv0 = new CEnvironment(testdev1);
    testenv0->properties.name = "default";
    testenv0->properties.active = false;
    testenv0->properties.currentSelection.type = user;
    testdev1->environments.append(testenv0);
    CInterface * testif0 = new CInterface(testenv0);
    testif0->properties.isStatic = true;
    testif0->properties.active = false;
    testif0->properties.ip = QHostAddress("192.168.0.5");
    testif0->properties.netmask = QHostAddress("255.255.255.0");
    testif0->properties.gateway = QHostAddress("192.168.0.1");
    testenv0->interfaces.append(testif0);
    mainwin.uiAddedDevice(testdev1);
    
    int exitcode = app.exec();
    
/*    mainwin.deviceManager.devices.removeAll(testdev0);
    mainwin.uiRemovedDevice(testdev0);
    delete testdev0;*/
    
    mainwin.deviceManager.devices.removeAll(testdev1);
    mainwin.uiRemovedDevice(testdev1);
    delete testdev1;
    
    return exitcode;
#else
    return app.exec();
#endif
}
