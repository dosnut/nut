
#ifndef _NUTS_HARDWARE_H
#define _NUTS_HARDWARE_H

#include <QObject>
#include <QString>
#include <QList>

namespace nuts {
	class HardwareManager;
};

namespace nuts {
	class HardwareManager : public QObject {
		Q_OBJECT
		signals:
			void gotCarrier(int ifIndex);
			void lostCarrier(int ifIndex);
	};
};

#endif
