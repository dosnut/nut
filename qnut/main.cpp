//#define QNUT_TESTING
#include <QtGui>
#include <QTranslator>
#include "connectionmanager.h"

using namespace qnut;

int main(int argc, char * argv[])
{
	common::init();
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;

    translator.load(QString("qnut_") + locale);
    app.installTranslator(&translator);
    app.setQuitOnLastWindowClosed(false);

//#ifdef QNUT_TESTING
/*    CDevice * testdev0 = new CDevice(NULL);
    testdev0->name = "eth0";
    testdev0->enabled = false;
    testdev0->type = ethernet;
    testdev0->activeEnvironment = 0;
    mainwin.deviceManager.devices.append(testdev0);
    CEnvironment * testenv1 = new CEnvironment(testdev0);
    testenv1->name = "default";
    testenv1->active = false;
    libnut_SelectConfig testconf;
    testconf.flags = user;
    testenv1->selectStatements.append(testconf);
    testdev0->environments.append(testenv1);
    mainwin.uiAddedDevice(testdev0);
    
    CDevice * testdev1 = new CDevice(NULL);
    testdev1->name = "wlan0";
    testdev1->enabled = true;
    testdev1->type = wlan;
    testdev1->activeEnvironment = 0;
    mainwin.deviceManager.devices.append(testdev1);
    CEnvironment * testenv0 = new CEnvironment(testdev1);
    testenv0->name = "default";
    testenv0->active = false;
    testenv0->selectStatements.append(testconf);
    testdev1->environments.append(testenv0);
    CInterface * testif0 = new CInterface(testenv0);
    testif0->isStatic = true;
    testif0->active = false;
    testif0->ip = QHostAddress("192.168.0.5");
    testif0->netmask = QHostAddress("255.255.255.0");
    testif0->gateway = QHostAddress("192.168.0.1");
    testenv0->interfaces.append(testif0);
    mainwin.uiAddedDevice(testdev1);*/
    
    //int exitcode = app.exec();
    
/*    mainwin.deviceManager.devices.removeAll(testdev0);
    mainwin.uiRemovedDevice(testdev0);
    delete testdev0;*/
    
/*    mainwin.deviceManager.devices.removeAll(testdev1);
    mainwin.uiRemovedDevice(testdev1);
    delete testdev1;*/
    
    //return exitcode;
//#else
    CConnectionManager mainwin;
    return app.exec();
//#endif
}
