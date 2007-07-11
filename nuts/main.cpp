
#include "device.h"
#include "hardware.h"
#include "sighandler.h"
#include "exception.h"
#include "log.h"

#include <QCoreApplication>
#include <iostream>

using namespace nuts;

int main(int argc, char* argv[]) {
	Log_Init(log, 1);
	Log_Init(err, 2);
	QCoreApplication app(argc, argv);
	SigHandler *sighandler;
	DeviceManager *devManager;
	try {
		sighandler = new SigHandler();
		devManager = new DeviceManager("test.config");
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
