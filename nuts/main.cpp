
#include "config.h"

#include <QApplication>

using namespace nuts;

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	Config config("test.config");
	return 1;
}
