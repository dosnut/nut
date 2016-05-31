#ifndef NUTSPROCESSMANAGER_H
#define NUTSPROCESSMANAGER_H

#pragma once

#include <memory>

#include <QObject>
#include <QBasicTimer>
#include <QProcess>
#include <QList>
#include <QPointer>

namespace nuts {
	class ProcessManager;
	class Process;
}

namespace nuts {
	enum class ShutdownProcessHandling {
		Detach,
		KillAndWait,
		KillAndDetach,
	};

	class Process final : public QObject {
		Q_OBJECT
	private:
		explicit Process(
			ProcessManager* parent,
			QProcess *process,
			ShutdownProcessHandling handling = ShutdownProcessHandling::KillAndDetach,
			int waitBeforeShutdownMsecs = 3000,
			int killTimeoutMsecs = -1);
	public:
		~Process();

	protected:
		void timerEvent(QTimerEvent *event) override;
		void detach();

	private slots:
		void finished(int exitCode, QProcess::ExitStatus exitStatus);
		void error(QProcess::ProcessError error);

	private:
		friend class ProcessManager;
		void shutdown();

	private:
		QPointer<QProcess> m_process;
		ShutdownProcessHandling m_handling;
		int m_waitBeforeShutdownMsecs;
		int m_killTimeoutMsecs;

		QBasicTimer m_normal_timer;
		QBasicTimer m_shutdown_timer;
	};

	class ProcessManager : public QObject
	{
		Q_OBJECT
	public:
		void startProgram(const QProcessEnvironment& env, const QString& program, const QStringList& args);

		void shutdown();

	signals:
		void finishedShutdown();

	private:
		friend class Process;
		void addProcess(Process* process);
		void removeProcess(Process* process);

	private:
		bool m_shutdown = false;
		QList<Process*> m_processList;
	};
}

#endif // NUTSPROCESSMANAGER_H
