/*
    CErrorCodeEvaluator
    Copyright (C) 2010  Oliver Groß <z.o.gross@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <QStringList>

#include "cerrorcodeevaluator.h"

CErrorCodeEvaluator::CErrorCodeEvaluator() {
	m_ErrorHash.reserve(16);
}

QString CErrorCodeEvaluator::evaluate(unsigned long int code) const {
	return m_ErrorHash.value(code);
}

void CErrorCodeEvaluator::evaluate(QStringList & target, unsigned long int & errorFlags) const {
	QHash<unsigned long int, QString>::const_iterator i;
	for (i = m_ErrorHash.constBegin(); i != m_ErrorHash.constEnd(); ++i) {
		if (errorFlags & i.key()) {
			target.append(i.value());
			errorFlags ^= i.key();
		}
	}
}

void CErrorCodeEvaluator::evaluate(QString & target, const QString & separator, unsigned long int & errorFlags) const {
	QHash<unsigned long int, QString>::const_iterator i;
	for (i = m_ErrorHash.constBegin(); i != m_ErrorHash.constEnd(); ++i) {
		if (errorFlags & i.key()) {
			if (!target.isEmpty())
				target.append(separator);

			target.append(i.value());
			errorFlags ^= i.key();
		}
	}
}

void CErrorCodeEvaluator::registerErrorCode(unsigned long int code, const QString & string) {
	if (m_ErrorHash.contains(code))
		m_ErrorHash[code] = string;
	else
		m_ErrorHash.insert(code, string);
}

void CErrorCodeEvaluator::unregisterErrorCode(unsigned long int code) {
	m_ErrorHash.remove(code);
}
