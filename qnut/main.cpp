#include <QtGui>
#include <QTranslator>
#include "connectionmanager.h"
#include "constants.h"

using namespace qnut;

int main(int argc, char * argv[])
{
	common::init();
	QApplication app(argc, argv);
	
	QString locale = QLocale::system().name();
	QTranslator translator;
	translator.load(QString(UI_DIR_TRANSLATIONS "qnut_") + locale);
	app.installTranslator(&translator);
	app.setQuitOnLastWindowClosed(false);
	
	{
		QDir workdir(QDir::toNativeSeparators(QDir::homePath()) + "/" UI_DIR_WORK);
		if (!workdir.exists()) {
			workdir.cdUp();
			workdir.mkdir(UI_DIR_WORK);
		}
	}
	
	CConnectionManager mainwin;
	return app.exec();
}
