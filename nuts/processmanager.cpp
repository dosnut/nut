#include "processmanager.h"

#include <QChildEvent>
#include <QDebug>

namespace nuts {
	Process::Process(ProcessManager* parent, QProcess* process, ShutdownProcessHandling handling, int waitBeforeShutdownMsecs, int killTimeoutMsecs)
		: QObject(parent)
		, m_process(process)
		, m_handling(handling)
		, m_waitBeforeShutdownMsecs(waitBeforeShutdownMsecs)
		, m_killTimeoutMsecs(killTimeoutMsecs)
	{
		if (m_killTimeoutMsecs > 0) {
			m_normal_timer.start(m_killTimeoutMsecs, this);
		}
		parent->addProcess(this);
		connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Process::finished);
		connect(m_process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &Process::error);
	}

	Process::~Process()
	{
		static_cast<ProcessManager*>(parent())->removeProcess(this);
	}

	void Process::timerEvent(QTimerEvent* event) {
		if (!m_process) return;

		if (event->timerId() == m_normal_timer.timerId()) {
			m_normal_timer.stop();
			m_process->terminate();
		} else if (event->timerId() == m_shutdown_timer.timerId()) {
			m_shutdown_timer.stop();
			switch (m_handling) {
			case ShutdownProcessHandling::KillAndDetach:
				m_process->terminate();
				detach();
				break;
			case ShutdownProcessHandling::Detach:
				detach();
				break;
			case ShutdownProcessHandling::KillAndWait:
				m_process->terminate();
				break;
			}
		}
	}

	void Process::detach() {
		if (m_process) {
			// leak m_process here - the destructor would block
			m_process->setParent(nullptr);
			m_process.clear();
		}
		deleteLater();
	}

	void Process::finished(int exitCode, QProcess::ExitStatus exitStatus)
	{
		if (!m_process) return;
		if (exitStatus == QProcess::CrashExit) {
			qWarning()
				<< "Process " << m_process->processId() << " crashed with exit code " << exitCode << "\n"
				<< "Command: " << m_process->program() << " " << m_process->arguments() << "\n"
				<< "Environment: " << m_process->environment();
		} else if (0 != exitCode) {
			qWarning()
				<< "Process " << m_process->processId() << " exited with non-zero exit code " << exitCode << "\n"
				<< "Command: " << m_process->program() << " " << m_process->arguments() << "\n"
				<< "Environment: " << m_process->environment();
		}
		m_process->deleteLater();
		m_process.clear();
		deleteLater();
	}

	void Process::error(QProcess::ProcessError error)
	{
		if (!m_process) return;
		switch (error) {
		case QProcess::Crashed:
			// wait for finished signal
			return;
		case QProcess::FailedToStart:
			qWarning()
				<< "Failed to start command: " << m_process->program() << " " << m_process->arguments() << "\n"
				<< "Environment: " << m_process->environment();
			break;
		default:
			qWarning()
				<< "Unknown error with process " << m_process->processId() << "\n"
				<< "Command: " << m_process->program() << " " << m_process->arguments() << "\n"
				<< "Environment: " << m_process->environment();
			break;
		}

		m_process->kill();
		m_process->deleteLater();
		m_process.clear();
		deleteLater();
	}

	void Process::shutdown() {
		m_shutdown_timer.start(m_waitBeforeShutdownMsecs, this);
	}

	void ProcessManager::startProgram(const QProcessEnvironment& env, const QString& program, const QStringList& args)
	{
		QProcess* process = new QProcess();
		process->setProcessEnvironment(env);
		process->start(program, args);
		new Process(this, process);
	}

	void ProcessManager::shutdown() {
		if (m_shutdown) return;
		m_shutdown = true;

		if (m_processList.empty()) {
			emit finishedShutdown();
			return;
		}

		// copy list
		auto pl = m_processList;
		for (auto p: pl) {
			p->shutdown();
		}
	}

	void ProcessManager::addProcess(Process* process)
	{
		if (m_shutdown) {
			qWarning("New process but already shutting down");
			process->shutdown();
		} else {
			m_processList.append(process);
		}
	}

	void ProcessManager::removeProcess(Process* process)
	{
		if (m_processList.removeOne(process) && m_shutdown && m_processList.empty()) emit finishedShutdown();
	}
}
