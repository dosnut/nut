#include <QCoreApplication>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <iostream>
#include <libnut/libnut_cli.h>
#include <QDBusConnectionInterface>
#include <QDBus>


void echo(QString str);
void echo(QStringList str);

typedef int (*Tcommand_function)(QStringList argv) ;

class Cnut_commands {
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
QList<Cnut_commands> nut_init_commands();

//Commandfunctions
//Commandtypes: 0=integer,1=String
//Gibt eine Liste von QVariants zurück, die nach dem Schema von command_tyes geparst wurden.
QList<QVariant> commandParser(QStringList str,QList<int> command_types);
int list_environments(QStringList argv);
int list_environments_dv(QStringList argv);
int set_environments(QStringList argv);
int set_environments_dv(QStringList argv);
int disable_interface(QStringList argv);
int enable_interface(QStringList argv);
int list_interfaces(QStringList argv);
int list_interfaces_dv(QStringList argv);
int list_environments(QStringList argv);
int list_environments(QStringList argv);
int add_user_interface(QStringList argv);
int add_user_interface(QStringList argv);
int list_environments(QStringList argv);