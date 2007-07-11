#include <iostream>
#include <"nutdbus.h">
#include <QDBus>
#include <QDBusConnectionInterface>


int main(int argc, char* argv[]) {
    cout << "Server testing program" << endl;
    //First register service:
    cout << "Register Service" << "SERVICE_ADDRESS" << endl;

    if (!QDBusConnectionInterface::isServiceRegistered("NUT_SERVICE_ADDRESS").value()) {
        echo("Registration failed. Please Check your settings");
        return 1;
    }
    cout << "Sending user signals" << endl;
    
    
}