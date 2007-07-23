#include <QCoreApplication>
#include <QList>
#include <iostream>
#include "../libnut/libnut_cli.h"
#include <"nutdbus.h">
#include <QDBusConnectionInterface>
#include <QDBus>
#include "nut_commandline.h"
#include "nut_interactive.h"
#include "nut_library.h"




//Interaktiver Modus mit TAB-Competion
void interactive_mode(QCoreApplication app) {
    QStringList commandlist;
    while (app.arguments().first() != "exit") {
        int command_index = commands.contains((Ccommands) app.arguments().first());
        if (command_index != -1) {
            commands[command_index].exec();
        }
        else {
            echo("Command not found");
            if (commandlist.empty()) {
                for (QList<Tcommands>::iterator i = commands.begin(); i != commands.end(); ++i) {
                    commandlist.append(*i.name);
                }
            }
            else {
                echo(commandlist(commands));
            }
        }
    }
}

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
