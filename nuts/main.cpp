
#include "device.h"
#include "hardware.h"
#include "sighandler.h"
#include "exception.h"
#include "log.h"

#include <QApplication>
#include <iostream>

using namespace nuts;

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SigHandler *sighandler;
	DeviceManager *devManager;
	HardwareManager *hwManager;
	try {
		sighandler = new SigHandler();
		devManager = new DeviceManager("test.config");
		hwManager = new HardwareManager();
	} catch (Exception &e) {
		err << "Initialize failed:" << endl
		          << "    " << e.what() << endl;
		return -1;
	}
	try {
		app.exec();
	} catch (Exception &e) {
		err << e.what() << endl;
		return -2;
	}
	try {
		delete hwManager;
		delete devManager;
		delete sighandler;
	} catch (Exception &e) {
		std::cerr << "Cleanup failed:" << std::endl
		          << "    " << e.what() << std::endl;
		return -3;
	}
	return 0;
}
