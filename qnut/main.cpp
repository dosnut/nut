#include <QtGui>
#include <QTranslator>

#include "connectionmanager.h"

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
    
    //testing
    CDevice * testdev = new CDevice(NULL);
    testdev->properties.name = "eth0";
    testdev->properties.enabled = false;
    mainwin.deviceManager.devices.append(testdev);
    mainwin.uiAddedDevice(testdev);
    //testing
    
    return app.exec();
}
