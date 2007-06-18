
#include "device.h"
#include "hardware.h"
#include "sighandler.h"

#include <QApplication>

using namespace nuts;

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SigHandler sighandler;
	DeviceManager *devManager = new DeviceManager("test.config");
	HardwareManager *hwManager = new HardwareManager();
	app.exec();
	delete hwManager;
	delete devManager;
	return 1;
}
