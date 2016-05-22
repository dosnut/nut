
#include "device.h"
#include "hardware.h"
#include "sighandler.h"
#include "exception.h"
#include "log.h"
#include "dbus.h"
#include "processmanager.h"

#include <QCoreApplication>
#include <iostream>

#include <libnutcommon/dbusmanager.h>
namespace nuts {
	int mainApp(int argc, char* argv[]) {
		QCoreApplication app(argc, argv);

		std::unique_ptr<libnutcommon::DBusManager> dbusManager;
		std::unique_ptr<SigHandler> sighandler;
		std::unique_ptr<DeviceManager> devManager;
		std::unique_ptr<ProcessManager> processManager;

// TODO: log when dbus connection got lost
//	log << "dbus has been stopped, please restart it";


		try {
			dbusManager.reset(new libnutcommon::DBusManager(libnutcommon::createDefaultDBusConnection));
			sighandler.reset(new SigHandler(/* quitOnSignal = */ false));
			processManager.reset(new ProcessManager());
			devManager.reset(new DeviceManager(argc > 1 ? argv[1] : "/etc/nuts/nuts.config", processManager.get()));
			auto dbusDevManager = new DBusDeviceManager(devManager.get());
			dbusDevManager->connectManager(dbusManager.get());

			QObject::connect(sighandler.get(), &SigHandler::gotQuitSignal, [&processManager, &devManager]() {
				if (devManager) devManager->shutdown();
				if (processManager) processManager->shutdown();
			});
			QObject::connect(processManager.get(), &ProcessManager::finishedShutdown, [](){
				QCoreApplication::exit();
			});

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
			processManager.reset();
			devManager.reset();
			sighandler.reset();
			dbusManager.reset();
		} catch (Exception &e) {
			err << "Cleanup failed:" << endl
					<< "    " << e.msg() << endl;
			return -3;
		}
		return 0;
	}
}

int main(int argc, char* argv[]) {
	libnutcommon::init();
	nuts::LogInit();
	int res = nuts::mainApp(argc, argv);
	nuts::LogDestroy();
	return res;
}
