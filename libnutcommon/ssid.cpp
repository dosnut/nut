#include "ssid.h"

#include <QDBusMetaType>
#include <QVector>

namespace libnutcommon {
	SSID SSID::fromRaw(quint8 const* mem, size_t len) {
		SSID result;
		result.m_data = QByteArray(reinterpret_cast<char const*>(mem), len);
		return result;
	}

	SSID SSID::fromRaw(QByteArray const& bytes) {
		return fromRaw(reinterpret_cast<quint8 const*>(bytes.constData()), bytes.length());
	}

	SSID SSID::fromQuotedString(QString const& str) {
		return fromRaw(unquoteString(str));
	}

	SSID SSID::fromHexString(const QString& str) {
		return fromRaw(QByteArray::fromHex(str.toUtf8()));
	}

	QByteArray const& SSID::data() const {
		return m_data;
	}

	QString SSID::quotedString() const {
		return quoteString(m_data);
	}

	bool SSID::needsQuoting() const {
		return stringNeedsQuoting(m_data);
	}

	QString SSID::hexString() const {
		return QString::fromLatin1(m_data.toHex());
	}

	QString SSID::autoQuoteHexString() const {
		if (needsQuoting()) {
			return "0x\"" +  hexString() + "\"";
		} else {
			return quotedString();
		}
	}

	static bool tryReadUtf8Char(quint32& result, size_t& bytes, quint8 const* mem, size_t pos, size_t len) {
		result = 0;
		bytes = 0;
		if (pos >= len) return false;
		result = mem[pos];
		if (0u == (result & 0x80u)) {
			bytes = 1;
		} else if (0xc0u == (result & 0xe0u)) {
			bytes = 2;
			result &= ~0xe0;
		} else if (0xe0u == (result & 0xf0u)) {
			bytes = 2;
			result &= ~0xf0;
		} else if (0xf0u == (result & 0xf8u)) {
			bytes = 4;
			result &= ~0xf8u;
		} else {
			return false;
		}
		if (pos + bytes > len) return false;
		size_t read_bytes = bytes;
		while (--read_bytes > 0) {
			if (0 != (mem[++pos] & 0xc0u)) return false;
			result = (result << 6) | (mem[pos] & ~0xc0u);
		}
		return true;
	}

	QByteArray unquoteString(QString const& str) {
		QVector<uint> strData = str.toUcs4();
		QByteArray result;
		result.reserve(strData.size());
		size_t len = strData.size();
		for (size_t pos = 0; pos < len; ++pos) {
			if ('\\' == strData.at(pos) && (pos + 1 < len)) {
				++pos;
				switch (strData.at(pos)) {
				case '\\':
				case '"':
				case '\'':
					result.append(strData.at(pos));
					break;
				case 'n':
					result.append('\n');
					break;
				case 'r':
					result.append('\r');
					break;
				case 't':
					result.append('\t');
					break;
				case 'e':
					result.append('\033');
					break;
				case 'x':
					if (pos + 1 >= len) {
						result.append("\\x", 2);
					} else {
						++pos;

						char value;

						char digit = strData.at(pos);
						if (digit >= '0' && digit <= '9') {
							value = digit - '0';
						} else if (digit >= 'a' && digit <= 'f') {
							value = digit - 'a' + 10;
						} else if (digit >= 'A' && digit <= 'F') {
							value = digit - 'A' + 10;
						} else {
							result.append("\\x", 2);
							--pos;
							break;
						}

						if (pos + 1 < len) {
							++pos;
							if (digit >= '0' && digit <= '9') {
								value = (value << 4) + digit - '0';
							} else if (digit >= 'a' && digit <= 'f') {
								value = (value << 4) + digit - 'a' + 10;
							} else if (digit >= 'A' && digit <= 'F') {
								value = (value << 4) + digit - 'A' + 10;
							} else {
								--pos;
							}
						}

						result.append(value);
					}
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					{
						int val = strData.at(pos) - '0';
						if ((pos + 1 < len) && strData.at(pos + 1) >= '0' && strData.at(pos + 1) <= '7')
							val = val * 8 + (strData.at(++pos) - '0');
						if ((pos + 1 < len) && strData.at(pos + 1) >= '0' && strData.at(pos + 1) <= '7')
							val = val * 8 + (strData.at(++pos) - '0');
						result.append(static_cast<char>(val));
					}
					break;
				default:
					result.append('\\');
					--pos;
					break;
				}
			} else {
				// append as UTF-8 encoded
				uint codepoint = strData.at(pos);
				result.append(QString::fromUcs4(&codepoint, 1).toUtf8());
			}
		}
		return result;
	}

	QString quoteString(QByteArray const& data) {
		QVector<uint> result;
		result.reserve(data.size());
		quint8 const* mem = reinterpret_cast<quint8 const*>(data.constData());
		size_t len = data.size();
		size_t pos = 0;
		while (pos < len) {
			switch (mem[pos]) {
			case '\"':
				result.append('\\');
				result.append('\"');
				++pos;
				break;
			case '\\':
				result.append('\\');
				result.append('\\');
				++pos;
				break;
			case '\033':
				result.append('\\');
				result.append('e');
				++pos;
				break;
			case '\n':
				result.append('\\');
				result.append('n');
				++pos;
				break;
			case '\r':
				result.append('\\');
				result.append('r');
				++pos;
				break;
			case '\t':
				result.append('\\');
				result.append('t');
				++pos;
				break;
			default:
				{
					quint32 codepoint;
					size_t bytes;
					if (tryReadUtf8Char(codepoint, bytes, mem, pos, len)) {
						if (codepoint < 0x10000 && QChar(static_cast<ushort>(codepoint)).isPrint()) {
							result.append(codepoint);
							pos += bytes;
							break;
						}
					}
					result << QString("\\x%1").arg(mem[pos], 2, 16, QChar('0')).toUcs4();
					++pos;
				}
				break;
			}
		}
		return QString::fromUcs4(result.constData(), result.size());
	}

	bool stringNeedsQuoting(QByteArray const& data) {
		quint8 const* mem = reinterpret_cast<quint8 const*>(data.constData());
		size_t len = data.size();
		size_t pos = 0;
		while (pos < len) {
			switch (mem[pos]) {
			case '\"':
			case '\\':
			case '\033':
			case '\n':
			case '\r':
			case '\t':
				// quote it
				return true;
			default:
				{
					quint32 codepoint;
					size_t bytes;
					if (tryReadUtf8Char(codepoint, bytes, mem, pos, len)) {
						if (codepoint < 0x10000 && QChar(static_cast<ushort>(codepoint)).isPrint()) {
							pos += bytes;
							break;
						}
					}
					// quote unprintable byte
					return true;
				}
				break;
			}
		}
		return false;
	}

	uint qHash(libnutcommon::SSID const& ssid) {
		return qHash(ssid.data());
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SSID const& ssid) {
		argument.beginStructure();
		argument << ssid.data();
		argument.endStructure();
		return argument;
	}

	QDBusArgument const& operator>>(QDBusArgument const& argument, SSID& ssid) {
		QByteArray rawSSID;
		argument.beginStructure();
		argument >> rawSSID;
		ssid = SSID::fromRaw(rawSSID);
		argument.endStructure();
		return argument;
	}

	// called by common.cpp: init()
	void ssid_init() {
		qRegisterMetaType<SSID>("libnutcommon::SSID");

		qDBusRegisterMetaType<SSID>();
	}
}
