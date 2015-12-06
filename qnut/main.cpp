#include <QtGui>
#include <QTranslator>
#include <QApplication>
#include <QCommandLineParser>

#include "cconnectionmanager.h"
#include "constants.h"

using namespace qnut;

int main(int argc, char * argv[])
{
	libnutcommon::init();
	QApplication app(argc, argv);
	QApplication::setApplicationVersion(libnutcommon::version());

	QString locale = QLocale::system().name();
	QTranslator translator;
	translator.load(QString(UI_PATH_TRANSLATIONS "qnut_") + locale);
	app.installTranslator(&translator);

	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::translate("main", "qnut - graphical interface for nut (a fancy network manager)"));
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption showOption(
		QStringList() << "s" << "show",
		QCoreApplication::translate("main", "Show UI, don't minimize on start."));
	parser.addOption(showOption);

	parser.process(app);

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
	if (parser.isSet(showOption)) mainwin.show();
	return app.exec();
}
