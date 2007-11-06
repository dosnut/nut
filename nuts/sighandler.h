//
// C++ Interface: sighandler
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _NUTS_SIGHANDLER_H
#define _NUTS_SIGHANDLER_H

#include <QObject>

namespace nuts {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class SigHandler : public QObject
	{
		Q_OBJECT
		private:
			bool quitOnSignal;
		private slots:
			void pipe_rcv();
			
		public:
			SigHandler(bool quitOnSignal = true);
			virtual ~SigHandler();
		
		signals:
			void gotSignal(int signum);
			void appQuit();
	};
};

#endif
