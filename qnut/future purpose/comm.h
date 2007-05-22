/*
 * Idee:
 *  Die Communicator Klasse wird zur Kommunikation verwendet und das
 *  Hauptprogramm erstellt eine Instanz dessen.
 *  Diese bekommt eine Instanz eines Kindes des Eventhandlers übergeben
 *  und kann dessen Funktionen bei den entsprechenden Ereignissen
 *  aufrufen.
 */

#ifndef COMM_H
#define COMM_H

#include <string>

using namespace std;

class CCommunicator {
private:
    //abstrakte Klasse für Eventhandler
    class CEventHandler {
    public:
        //Ereignisse - Device
        virtual void eventDevAppear(int id, string name, deviceType type, int envcount, ) = 0;
        virtual void eventDevLost(int id) = 0;
        virtual void eventDevUp(int id) = 0;
        virtual void eventDevPreDown(int id) = 0;
        virtual void eventDevPostDown(int id) = 0;

        //Ereignisse - Environment
        virtual void eventEnvPreSet(int id) = 0;
        virtual void eventEnvPostSet(int id) = 0;
    };

    CEventHandler * eventHandler;

public:
    //Befehle - Device
    void cmdDevSetUp(int id);
    void cmdDevSetDown(int id);
    void cmdDevSetEnv(int devId, int envId);

    CCommunicator(CEventHandler * handler);
}

#endif
