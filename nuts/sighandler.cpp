//
// C++ Implementation: sighandler
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "sighandler.h"

extern "C" {
// pipe
#include <unistd.h>
// signal
#include <signal.h>
// fcntl
#include <fcntl.h>
};

#include <iostream>

#include <QSocketNotifier>
#include <QCoreApplication>

namespace nuts {
	static int pipefd[2];

	static void sig2pipe(int sig) {
		write(pipefd[1], (char*) &sig, sizeof(sig));
	}

	SigHandler::SigHandler(bool quitOnSignal)
	: quitOnSignal(quitOnSignal) {
		if (pipefd[1] != 0) return;
		if (pipe(pipefd) != 0) return;
		fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
		fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);
		
		signal(SIGTERM, sig2pipe);
		signal(SIGINT, sig2pipe);

		QSocketNotifier *r = new QSocketNotifier(pipefd[0], QSocketNotifier::Read, this);
		connect(r, SIGNAL(activated(int)), this, SLOT(pipe_rcv()));
	}
	
	SigHandler::~SigHandler() {
		close(pipefd[0]);
		close(pipefd[1]);
		pipefd[0] = pipefd[1] = 0;
	}
	void SigHandler::pipe_rcv() {
		int signum = 0;
		if (read(pipefd[0], &signum, sizeof(signum))) {
			std::cout << "Received signal: " << signum << std::endl;
			emit gotSignal(signum);
			switch (signum) {
				case SIGTERM:
				case SIGINT:
					if (quitOnSignal)
						QCoreApplication::quit();
			}
		}
	}

};
