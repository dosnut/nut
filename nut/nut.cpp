#include <QCoreApplication>
#include <QList>
#include <iostream>
#include <"../libnut/libnut_cli.h">
#include <"nutdbus.h">
#include <QDBusConnectionInterface>
#include <QDBus>


typedef int (*Tcommand_function)(QStringList argv) ;

class Ccommands {
    public:
        Ccommands(QString name, short p_count = 0,Tcommand_function exec = null)
        : name(name), p_count(p_count), exec(exec) {}
        QString name;
        short p_count;
        Tcommand_function exec;
        bool operator==(Ccommands c1, Ccommands c2) {
            return (c1.name == c2.name);
        }
};
QList<Tcommands> commands;

void echo(QStringList str) {
    for (QStringList::iterator i = str.begin(); i != str.end(); ++i) {
        cout << (*i).toUtf8().constData() << endl;
    }
}
void echo(QString str) {
    cout << str.toUtf8().constData() << endl;
}

//Interactive mode with (hopefully) tab-completion
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
    //Possible commands:
    //Client:
    CCommand command("list_environments",0,list_environments); //ohne device
    commands.append(command);
    CCommand command("list_environments",1,list_environments_dv); //mit device
    commands.append(command);
    CCommand command("set_environment",1,set_environments); //ohne device
    commands.append(command);
    CCommand command("set_environment",2,set_environments_dv); //mit device
    commands.append(command);
    CCommand command("disable_interface",1,disable_interface);
    commands.append(command);
    CCommand command("enable_interface",1,enable_interface);
    commands.append(command);
    CCommand command("list_interfaces",0,list_interfaces); //ohne Device
    commands.append(command);
    CCommand command("list_interfaces",1,list_interfaces_dv); //mit Device
    commands.append(command);
    CCommand command("enable_device",1,list_environments);
    commands.append(command);
    CCommand command("disable_device",1,list_environments);
    commands.append(command);

    //User interaction
    //add_user_interface(QString Device, QString env = "user", int count = 0)
    CCommand command("add_user_interface",2,add_user_interface); //fügt interface in userenv hinzu
    commands.append(command);
    CCommand command("add_user_interface",3,add_user_interface); //fügt interface in userenv hinzu
    commands.append(command);
    //set_user_interface(QString Device, QString env = "user", int count = 0, QStringList param)
    //possible commands:
    CCommand command("set_user_interface",1,list_environments); //setzt die Interfaces im usermodus
    commands.append(command);

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
        noninteractive_mode(app);
    }
    return 0;
}
