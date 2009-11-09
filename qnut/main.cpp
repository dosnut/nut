#include <QtGui>
#include <QTranslator>
#include "connectionmanager.h"
#include "constants.h"

using namespace qnut;

int main(int argc, char * argv[])
{
	libnutcommon::init();
	QApplication app(argc, argv);
	
	QString locale = QLocale::system().name();
	QTranslator translator;
	translator.load(QString(UI_PATH_TRANSLATIONS "qnut_") + locale);
	app.installTranslator(&translator);
	app.setQuitOnLastWindowClosed(false);
	
#ifndef QNUT_SETTINGS_NOCOMPAT
	{
		QDir workdir(UI_PATH_WORK);
		if (!workdir.exists()) {
			workdir.cdUp();
			workdir.mkdir(UI_DIR_WORK);
		}
	}
#endif
	
	QDir::setCurrent(QDir::homePath());
	
	CConnectionManager mainwin;
	return app.exec();
}
