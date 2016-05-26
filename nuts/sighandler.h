#ifndef _NUTS_SIGHANDLER_H
#define _NUTS_SIGHANDLER_H

#pragma once

#include <QObject>

namespace nuts {
	class SigHandler final : public QObject
	{
		Q_OBJECT
	public:
		explicit SigHandler(bool quitOnSignal = true);
		~SigHandler();

	signals:
		void gotSignal(int signum);
		void gotQuitSignal();

	private slots:
		void pipe_rcv();

	private:
		bool quitOnSignal;
	};
}

#endif /* _NUTS_SIGHANDLER_H */
