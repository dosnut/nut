
#include "device.h"
#include "hardware.h"
#include "sighandler.h"
#include "exception.h"
#include "log.h"
#include "dbus.h"

#include <QCoreApplication>
#include <iostream>

using namespace nuts;

int mainApp(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);
	SigHandler *sighandler;
	DeviceManager *devManager;
	
	try {
		sighandler = new SigHandler();
		devManager = new DeviceManager(argc > 1 ? argv[1] : "/etc/nuts/nuts.config");
		devManager->dbus_devMan = new DBusDeviceManager(devManager);
	} catch (Exception &e) {
		err << "Initialize failed:" << endl
		          << "    " << e.msg() << endl;
		return -1;
	}
	try {
		app.exec();
	} catch (Exception &e) {
		err << e.msg() << endl;
		return -2;
	}
	try {
		delete devManager;
		delete sighandler;
	} catch (Exception &e) {
		err << "Cleanup failed:" << endl
				<< "    " << e.msg() << endl;
		return -3;
	}
	return 0;
}

int main(int argc, char* argv[]) {
	common::init();
	LogInit();
	int res = mainApp(argc, argv);
	LogDestroy();
	return res;
}
