#include <QCoreApplication>
#include <QList>
#include <iostream>
#include <libnut/libnut_cli.h>
#include <QDBusConnectionInterface>
#include <QDBus>


//Interaktiver Modus mit TAB-Competion
int main(int argc, char* argv[]) {


    QCoreApplication app (argc,argv);
    //Check if nuts is running:
    if (!QDBusConnectionInterface::isServiceRegistered("NUT_SERVICE_ADDRESS").value()) {
        echo("Service nuts not found. Please start nut's server component");
        return 1;
    }
    if (0 < argc) {
        interactive_mode(app);
    }
    else {
        commandline_mode(app);
    }
    return 0;
}
