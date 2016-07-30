#ifndef NUT_COMMON_SSID_H
#define NUT_COMMON_SSID_H

#pragma once

#include <QString>
#include <QDBusArgument>
#include <QHash>

namespace libnutcommon {
	class SSID {
	public:
		explicit SSID() noexcept {
		}

		static SSID fromRaw(quint8 const* mem, size_t len);
		static SSID fromRaw(QByteArray const& bytes);
		static SSID fromQuotedString(QString const& str);
		static SSID fromHexString(QString const& str);

		QByteArray const& data() const;
		QString quotedString() const;
		bool needsQuoting() const;
		QString hexString() const;
		// choose some representation for display, don't use in protocols/parsing
		QString autoQuoteHexString() const;

		friend bool operator<(SSID const& a, SSID const& b) {
			return a.m_data < b.m_data;
		}
		friend bool operator==(SSID const& a, SSID const& b) {
			return a.m_data == b.m_data;
		}
		friend bool operator!=(SSID const& a, SSID const& b) {
			return a.m_data != b.m_data;
		}

	private:
		QByteArray m_data;
	};

	QByteArray unquoteString(QString const& str);
	QString quoteString(QByteArray const& data);
	bool stringNeedsQuoting(QByteArray const& data);

	uint qHash(libnutcommon::SSID const& ssid);

	QDBusArgument& operator<<(QDBusArgument& argument, SSID const& ssid);
	QDBusArgument const& operator>>(QDBusArgument const& argument, SSID& ssid);
}

Q_DECLARE_METATYPE(libnutcommon::SSID)

#endif
