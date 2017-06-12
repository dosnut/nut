#ifndef LIBNUTWIRELESS_CWIRELESS_H
#define LIBNUTWIRELESS_CWIRELESS_H

#pragma once

#include <memory>

#include <QObject>

#ifndef NUT_NO_WIRELESS
namespace libnutwireless {
	class CWpaSupplicant;
	class CWirelessHW;

	class CWireless final: public QObject {
		Q_OBJECT
	public:
		explicit CWireless(QObject* parent, QString ifname);
		~CWireless();

		void open();
		void close();
		CWpaSupplicant* getWpaSupplicant() { return m_wpa_supplicant.get(); }
		CWirelessHW* getHardware() { return m_wireless_hw.get(); }

	public slots:
		void scan();

	signals:
		void message(QString const& msg);

	private:
		QString m_ifname;
		std::unique_ptr<CWpaSupplicant> m_wpa_supplicant;
		std::unique_ptr<CWirelessHW> m_wireless_hw;
	};
}
#endif
#endif /* LIBNUTWIRELESS_CWIRELESS_H */
